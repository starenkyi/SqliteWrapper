#include "../include/connection.h"

#include <cstring>

#include "../include/sqlite3.h"
#include "../include/statement.h"


std::mutex Connection::_mutex;

std::atomic<Connection::ThreadMode>
Connection::_libThreadMode(sqlite3_threadsafe()
                           ? Connection::ThreadMode::Serialized
                           : Connection::ThreadMode::SingleThread);

std::atomic_uint Connection::_openedConn(0);

Connection::Connection(const Connection::OpenMode  openMode,
                       const Connection::CacheMode cacheMode)
    : _db(NULL),
      _openMode(openMode),
      _cacheMode(cacheMode),
      _lastResultCode(-1)
{}

Connection::Connection(const char* const dbName,
                       const OpenMode    openMode,
                       const CacheMode   cacheMode)
    : _db(NULL),
      _dbName(dbName),
      _openMode(openMode),
      _cacheMode(cacheMode),
      _lastResultCode(-1)
{}

Connection::Connection(const std::string& dbName,
                       const OpenMode     openMode,
                       const CacheMode    cacheMode)
    : _db(NULL),
      _dbName(dbName),
      _openMode(openMode),
      _cacheMode(cacheMode),
      _lastResultCode(-1)
{}

Connection::~Connection()
{
    close();
}

void Connection::close() noexcept
{
    // check if connection is opened
    if (_db) {
        // close connection
        sqlite3_close_v2(_db);
        _db = NULL;

        // clear error code and message
        _lastResultCode = -1;
        _openErrorMsg.clear();

        // increase openned connection count
        _openedConn.fetch_sub(1, std::memory_order_relaxed);
    }
}

bool Connection::commit() noexcept
{
    return execute("COMMIT");
}

bool Connection::execute(const char* const query) noexcept
{
    // execute query if connection is opened
    if (_db) {
        _lastResultCode = sqlite3_exec(_db, query, NULL, NULL, NULL);
    }

    // return result of query execution
    return _lastResultCode == SQLITE_OK;
}

bool Connection::execute(const std::string& query) noexcept
{
    return execute(query.c_str());
}

std::string Connection::databaseName() const noexcept
{
    // return database file name
    return _dbName;
}

bool Connection::open()
{
    std::lock_guard<std::mutex> lock(_mutex);

    // check connection is opened
    if (_db) {
        // connection is already opened
        return true;
    } else {

        // try open database
        switch (_openMode) {
        case OpenMode::Temporary:
            _lastResultCode = openTemporaryDb();
            break;
        case OpenMode::InMemory:
            _lastResultCode = openTemporaryDb();
        default:
            _lastResultCode = openRegularDb();
            break;
        }

        // chek is connection opened
        if (_lastResultCode == SQLITE_OK) {
            _openedConn.fetch_add(1, std::memory_order_release);
        } else {

            // read and save last error
            const char* errStr = sqlite3_errmsg(_db);
            _openErrorMsg = errStr;

            // release sqlite3 pointer
            sqlite3_close_v2(_db);
            _db = NULL;
        }
    }

    return _lastResultCode == SQLITE_OK;
}

bool Connection::isOpen() const noexcept
{
    return _db;
}

std::string Connection::lastError() const
{
    std::string result;

    // try get error
    if (_db) {
        const char* errStr = sqlite3_errmsg(_db);
        if (errStr) {
            result = errStr;
        }
    } else {
        result = _openErrorMsg;
    }

    return result;
}

int64_t Connection::lastInsertRowId() const noexcept
{
    return (_db) ? sqlite3_last_insert_rowid(_db) : 0;
}

int Connection::lastResultCode() const noexcept
{
    return _lastResultCode;
}

Statement Connection::prepare(const char* const query,
                              const int         length) noexcept
{
    if (_db) {
        sqlite3_stmt *stmt;
        if ((_lastResultCode
             = sqlite3_prepare_v2(_db, query, length,
                                  &stmt, NULL)) == SQLITE_OK) {
            return Statement(stmt);
        }
    }

    return Statement();
}

Statement Connection::prepare(const std::string& query) noexcept
{
    return prepare(query.c_str(), query.length());
}

double Connection::readDouble(const std::string& query,
                              int*               resultCode) noexcept
{
    double result = 0.0;

    // try read double
    const int code = readValue(query, [&result] (sqlite3_stmt* stmt) -> void {
        result = sqlite3_column_double(stmt, 0);
    });

    // save result code, if pointer is valid
    if (resultCode) {
        *resultCode = code;
    }

    // return result
    return result;
}

int64_t Connection::readInt64(const std::string& query,
                              int*               resultCode) noexcept
{
    int64_t result = 0;

    // try read int64 and return result
    const int code = readValue(query, [&result] (sqlite3_stmt* stmt) -> void {
        result = sqlite3_column_int64(stmt, 0);
    });

    // save result code, if pointer is valid
    if (resultCode) {
        *resultCode = code;
    }

    // return result
    return result;
}

std::string Connection::readString(const std::string& query,
                                   int*               resultCode)
{
    std::string result;

    // try read string and return result
    const int code = readValue(query, [&result] (sqlite3_stmt* stmt) -> void {
        result.assign(reinterpret_cast<const char*>
                      (sqlite3_column_text(stmt, 0)),
                      sqlite3_column_bytes(stmt, 0));
    });

    // save result code, if pointer is valid
    if (resultCode) {
        *resultCode = code;
    }

    // return result
    return result;
}

std::u16string Connection::readString16(const std::string& query,
                                        int*               resultCode)
{
    std::u16string result;

    // try read string and return result
    const int code = readValue(query, [&result] (sqlite3_stmt* stmt) -> void {
        result = reinterpret_cast<const char16_t*>
            (sqlite3_column_text16(stmt, 0));
    });

    // save result code, if pointer is valid
    if (resultCode) {
        *resultCode = code;
    }

    // return result
    return result;
}

bool Connection::rollback() noexcept
{
    return execute("ROLLBACK");
}

void Connection::setDbName(const std::string& dbPath)
{
    // check if connection is open and assign value
    if (!_db) {
        _dbName = dbPath;
    }
}

bool Connection::transaction() noexcept
{
    return execute("BEGIN");
}

Connection& Connection::operator=(Connection&& connection) noexcept
{
    if (this != &connection) {
        close();

        // move assign object vars
        _db = connection._db;
        _dbName = std::move(connection._dbName);
        _openErrorMsg = std::move(connection._dbName);
        _openMode = connection._openMode;
        _cacheMode = connection._cacheMode;

        // reset moved object to default value
        connection._db = NULL;
        connection._dbName.clear();
        connection._openErrorMsg.clear();
    }

    return *this;
}

int Connection::setDefaultThreadMode(const ThreadMode value)
{
    // check opened connection count
    if (_openedConn > 0) {
        return -1;
    }

    std::lock_guard<std::mutex> gulockard(_mutex);

    // try configure thread mode
    const int resultCode = tryConfigThreadMode(configOptionFor(value));

    // if success, change default thread mode
    if (resultCode == SQLITE_OK) {
        _libThreadMode.store(value, std::memory_order_release);
    }

    // return result
    return resultCode;
}

Connection::ThreadMode Connection::defaultThreadMode() noexcept
{
    return _libThreadMode.load(std::memory_order_acquire);
}

int Connection::openedConnNumber() noexcept
{
    return _openedConn.load(std::memory_order_acquire);
}

int Connection::getOpenFlags() const noexcept
{
    int resFlags = 0;

    // set thread mode
    switch (_libThreadMode.load(std::memory_order_acquire)) {
    case ThreadMode::Serialized:
        resFlags |= SQLITE_OPEN_FULLMUTEX;
        break;
    case ThreadMode::MultiThread:
        resFlags |= SQLITE_OPEN_NOMUTEX;
        break;
    case ThreadMode::SingleThread:
    default:
        break;
    }

    // set cache mode
    switch (_cacheMode) {
    case CacheMode::Private:
        resFlags |= SQLITE_OPEN_PRIVATECACHE;
        break;
    case CacheMode::Shared:
        resFlags |= SQLITE_OPEN_SHAREDCACHE;
        break;
    default:
        break;
    }

    // set uri mode
    switch (_openMode) {
    case OpenMode::ReadOnly:
        resFlags |= SQLITE_OPEN_READONLY;
        break;
    case OpenMode::ReadWrite:
        resFlags |= SQLITE_OPEN_READWRITE;
        break;
    case OpenMode::ReadWriteCreate:
    default:
        resFlags |= SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        break;
    }

    return resFlags;
}

int Connection::openInMemoryDb()
{
    // build URI string
    std::string uriStr("file:");
    if (_dbName.empty()) {
        uriStr += ":memory:?cache=";
    } else {
        uriStr += _dbName + "?mode=memory&cache=";
    }

    uriStr += (_cacheMode == CacheMode::Private) ? "private" : "shared";

    // try open in-memory db and return result
    return _lastResultCode = sqlite3_open_v2(uriStr.c_str(), &_db,
                                             SQLITE_OPEN_URI, NULL);
}

int Connection::openRegularDb()
{
    return _lastResultCode = sqlite3_open_v2(_dbName.c_str(), &_db,
                                             getOpenFlags(), NULL);
}

int Connection::openTemporaryDb()
{
    if (!_dbName.empty()) {
        _dbName.clear();
    }
    return _lastResultCode = sqlite3_open_v2("", &_db, getOpenFlags(), NULL);
}

int Connection::readValue(const std::string&                  query,
                          std::function<void (sqlite3_stmt*)> readLambda)
{
    // check connection
    if (_db) {
        // try prepare statement
        sqlite3_stmt *stmt;
        _lastResultCode = sqlite3_prepare_v2(_db, query.c_str(),
                                            query.length(), &stmt, NULL);

        // if success, try read data (or set the error code)
        if (_lastResultCode == SQLITE_OK) {
            int resultCode;   // code for return

            if (sqlite3_column_count(stmt)) {
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    // check value type and read it (if it is not NULL)
                    if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
                        readLambda(stmt);
                        resultCode = ReadSuccess;
                    } else {
                        resultCode = NullValue;
                    }
                } else {
                    resultCode = EmptyData;
                }
            } else {
                resultCode = NoData;
            }

            // delete prepared statement and return result code
            sqlite3_finalize(stmt);
            return resultCode;
        }
    }

    // return error code
    return _lastResultCode;
}

int Connection::configOptionFor(const ThreadMode value) noexcept
{
    int result;

    switch (value) {
    case Connection::ThreadMode::MultiThread:
        result = SQLITE_CONFIG_MULTITHREAD;
        break;
    case Connection::ThreadMode::SingleThread:
        result = SQLITE_CONFIG_SINGLETHREAD;
        break;
    default:
        result = SQLITE_CONFIG_SERIALIZED;
        break;
    }

    return result;
}

int Connection::tryConfigThreadMode(const int option) noexcept
{
    if (_openedConn > 0) {
        return -1;
    }

    // try configure sqlite3 library
    int resultCode = sqlite3_config(option);

    // if sqlite3 library is initialized, try shutdown and configure it
    if (resultCode == SQLITE_MISUSE
            && (resultCode = sqlite3_shutdown()) == SQLITE_OK) {
        resultCode = sqlite3_config(option);
    }

    // return result
    return resultCode;
}
