#include "../include/statement.h"

// disable asserts in non-debug mode
#ifndef DEBUG
#define NDEBUG
#endif

#include <cassert>
#include <cstring>

#include "../include/sqlite3.h"


Statement::Statement() noexcept
    : _statement(NULL),
      _db(NULL),
      _columnCount(0),
      _type(Type::Undefined)
{}

Statement::Statement(Statement&& statement) noexcept
    : _statement(statement._statement),
      _db(statement._db),
      _columnCount(statement._columnCount),
      _type(statement._type)
{
    statement.reset();
}

Statement::~Statement() noexcept {
    sqlite3_finalize(_statement);
}

bool Statement::bindBlob(const int         index,
                         const void* const value,
                         const int         bytes) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_blob(_statement, index, value,
                             bytes, SQLITE_STATIC) == SQLITE_OK;
}

bool Statement::bindBlobCopy(const int         index,
                             const void* const value,
                             const int         bytes) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_blob(_statement, index, value,
                             bytes, SQLITE_TRANSIENT) == SQLITE_OK;
}

bool Statement::bindBool(const int  index,
                         const bool value) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_int(_statement, index, value) == SQLITE_OK;
}

bool Statement::bindCStr(const int         index,
                         const char* const value,
                         const int         bytes) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_text(_statement, index, value,
                             bytes, SQLITE_STATIC) == SQLITE_OK;
}

bool Statement::bindCStr16(const int         index,
                           const void* const value,
                           const int         bytes) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_text16(_statement, index, value,
                               bytes, SQLITE_STATIC) == SQLITE_OK;
}

bool Statement::bindCStrCopy(const int         index,
                             const char* const value,
                             const int         bytes) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_text(_statement, index, value,
                             bytes, SQLITE_TRANSIENT) == SQLITE_OK;
}

bool Statement::bindCStr16Copy(const int         index,
                               const void* const value,
                               const int         bytes) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_text16(_statement, index, value,
                               bytes, SQLITE_TRANSIENT) == SQLITE_OK;
}

bool Statement::bindDouble(const int    index,
                          const double value) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_double(_statement, index, value) == SQLITE_OK;
}

bool Statement::bindInt(const int index,
                        const int value) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_int(_statement, index, value) == SQLITE_OK;
}

bool Statement::bindInt64(const int     index,
                          const int64_t value) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_int64(_statement, index, value) == SQLITE_OK;
}

bool Statement::bindNull(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_null(_statement, index) == SQLITE_OK;
}

bool Statement::bindString(const int          index,
                           const std::string& value) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_text(_statement, index, value.c_str(),
                             value.length(), SQLITE_STATIC) == SQLITE_OK;
}

bool Statement::bindString16(const int             index,
                             const std::u16string& value) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_text16(_statement, index, value.c_str(),
                               value.length() << 1, SQLITE_STATIC) == SQLITE_OK;
}

bool Statement::bindStringCopy(const int          index,
                               const std::string& value) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_text(_statement, index, value.c_str(),
                             value.length(), SQLITE_TRANSIENT) == SQLITE_OK;
}

bool Statement::bindString16Copy(const int             index,
                                 const std::u16string& value) const noexcept
{
    assert(_statement != NULL);
    assert(index > 0);

    return sqlite3_bind_text16(_statement, index, value.c_str(),
                               value.length() << 1, SQLITE_TRANSIENT)
            == SQLITE_OK;
}

int32_t Statement::byteLength(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    return sqlite3_column_bytes(_statement, index);
}

int32_t Statement::byteLength16(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    return sqlite3_column_bytes16(_statement, index);
}

void Statement::clear() noexcept
{
    if (_statement) {
        sqlite3_finalize(_statement);
        reset();
    }
}

void Statement::clearBindings() const noexcept
{
    assert(_statement != NULL);

    sqlite3_clear_bindings(_statement);
}

int Statement::columnCount() const noexcept
{
    assert(_statement != NULL);

    return _columnCount;
}

int Statement::columnType(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    return sqlite3_column_type(_statement, index);
}

bool Statement::execute() const noexcept
{
    assert(_statement != NULL);

    const bool result = (sqlite3_step(_statement) == SQLITE_DONE);
    if (result) {
        sqlite3_reset(_statement);
    }

    return result;
}

std::string Statement::expandedQuery() const
{
    assert(_statement != NULL);

    std::string result;
    char* str = sqlite3_expanded_sql(_statement);
    if (str) {
        result = str;
    }

    sqlite3_free(str);
    return result;
}

std::pair<const unsigned char*, int>
Statement::getBlob(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    const unsigned char* result = reinterpret_cast<const unsigned char*>
            (sqlite3_column_blob(_statement, index));

    return std::pair<const unsigned char*, int>
    {result ? result : nullptr, sqlite3_column_bytes(_statement, index)};
}

std::pair<unsigned char*, int> Statement::getBlobCopy(const int index) const
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    const void* const temp = sqlite3_column_blob(_statement, index);
    const int bytes = sqlite3_column_bytes(_statement, index);
    unsigned char* result = nullptr;
    if (bytes) {
        result = new unsigned char[bytes];
        memcpy(result, temp, bytes);
    }

    return std::pair<unsigned char*, int> {result, bytes};
}

bool Statement::getBool(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    return sqlite3_column_int(_statement, index);
}

std::pair<const char*, int> Statement::getCStr(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    const char* result = reinterpret_cast<const char*>
            (sqlite3_column_text(_statement, index));

    return std::pair<const char*, int>
    {result ? result : nullptr, sqlite3_column_bytes(_statement, index)};
}

std::pair<const char16_t*, int>
Statement::getCStr16(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    const char16_t* result = reinterpret_cast<const char16_t*>
            (sqlite3_column_text(_statement, index));

    return std::pair<const char16_t*, int>
    {result ? result : nullptr, sqlite3_column_bytes(_statement, index)};
}

std::pair<char*, int>  Statement::getCStrCopy(const int index) const
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    const void* const from = sqlite3_column_text(_statement, index);
    const int bytes = sqlite3_column_bytes(_statement, index);
    char* result = nullptr;
    if (from) {
        result = new char[bytes + 1];
        memcpy(result, from, bytes + sizeof(char));
    }

    return std::pair<char*, int> {result, bytes};
}

std::pair<char16_t*, int> Statement::getCStr16Copy(const int index) const
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    const void* const from = sqlite3_column_text16(_statement, index);
    const int bytes = sqlite3_column_bytes16(_statement, index);
    char16_t* result = nullptr;
    if (from) {
        result = new char16_t[bytes + 1];
        memcpy(result, from, bytes + sizeof(char16_t));
    }

    return std::pair<char16_t*, int> {result, bytes};
}

double Statement::getDouble(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    return sqlite3_column_double(_statement, index);
}

float Statement::getFloat(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    return sqlite3_column_double(_statement, index);
}

int Statement::getInt(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    return sqlite3_column_int(_statement, index);
}

int64_t Statement::getInt64(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    return sqlite3_column_int64(_statement, index);
}

std::string Statement::getString(const int index) const
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    std::string result;

    // try read data
    const char* const ptr = reinterpret_cast<const char*>
            (sqlite3_column_text(_statement, index));
    if (ptr) {
        result.assign(ptr, sqlite3_column_bytes(_statement, index));
    }

    return result;
}

std::u16string Statement::getString16(const int index) const
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    std::u16string result;

    // try read data
    const char16_t* const ptr = reinterpret_cast<const char16_t*>
            (sqlite3_column_text16(_statement, index));
    if (ptr) {
        result.assign(ptr, sqlite3_column_bytes16(_statement, index) >> 1);
    }

    return result;
}

bool Statement::isNull(const int index) const noexcept
{
    assert(_statement != NULL);
    assert(_type == Statement::Select);
    assert(index >= 0);
    assert(index < _columnCount);

    return sqlite3_column_type(_statement, index) == SQLITE_NULL;
}

bool Statement::isValid() const noexcept
{
    return _statement;
}

std::string Statement::lastError() const
{
    assert(_db != NULL);

    std::string result;

    const char* str = sqlite3_errmsg(_db);
    if (str) {
        result.assign(str);
    }

    return result;
}

std::u16string Statement::lastError16() const
{
    assert(_db != NULL);

    std::u16string result;

    const void* str = sqlite3_errmsg16(_db);
    if (str) {
        result.assign(reinterpret_cast<const char16_t*>(str));
    }

    return result;
}

int Statement::lastErrorCode() const noexcept
{
    assert(_db != NULL);

    return sqlite3_errcode(_db);
}

int64_t Statement::lastInsertRowId() const noexcept
{
    assert(_db != NULL);

    return sqlite3_last_insert_rowid(_db);
}

bool Statement::next() const noexcept
{
    assert(_statement != NULL);
    assert(_type == Type::Select);

    return sqlite3_step(_statement) == SQLITE_ROW;
}

std::string Statement::query() const
{
    assert(_statement != NULL);

    std::string result;

    const char* str = sqlite3_sql(_statement);
    if (str) {
        result.assign(str);
    }

    return result;
}

Statement::Type Statement::type() const noexcept
{
    return _type;
}

Statement& Statement::operator=(Statement&& statement) noexcept
{
    if (this != &statement) {
        clear();

        _statement = statement._statement;
        _db = statement._db;
        _columnCount = statement._columnCount;
        _type = statement._type;

        statement.reset();
    }
    return *this;
}

void Statement::reset() noexcept
{
    _statement = NULL;
    _db = NULL;
    _columnCount = 0;
    _type = Type::Undefined;
}

Statement::Statement(sqlite3_stmt* statement) noexcept
    : _statement(statement ? statement : NULL),
      _db(statement ? sqlite3_db_handle(statement) : NULL),
      _columnCount(statement ? sqlite3_column_count(statement) : 0),
      _type(statement ? (sqlite3_stmt_readonly(statement)
                         ? Type::Select : Type::NonSelect) : Type::Undefined)
{}
