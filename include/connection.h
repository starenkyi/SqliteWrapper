#ifndef SQLITE_CONN_H
#define SQLITE_CONN_H

#include <atomic>
#include <functional>
#include <mutex>
#include <string>

#include "statement.h"

struct sqlite3;
struct sqlite3_stmt;

class Connection
{

public:

    enum ReadResult : int {
        ReadSuccess = 0,
        NoData = -1,
        EmptyData = -2,
        NullValue = -3
    };

    enum class CacheMode : uint8_t {
        Private = 0,
        Shared
    };

    enum class OpenMode : uint8_t {
        ReadWriteCreate = 0,
        ReadWrite,
        ReadOnly,
        Temporary,
        InMemory
    };

    enum class ThreadMode : uint8_t {
        Serialized = 0,
        MultiThread,
        SingleThread
    };

    static constexpr CacheMode defaultCacheMode { CacheMode::Private };
    static constexpr OpenMode  defaultOpenMode { OpenMode::ReadWriteCreate };

    Connection(const OpenMode    openMode = defaultOpenMode,
               const CacheMode   cacheMode = defaultCacheMode);

    Connection(const char* const dbName,
               const OpenMode    openMode = defaultOpenMode,
               const CacheMode   cacheMode = defaultCacheMode);

    Connection(const std::string& dbName,
               const OpenMode     openMode = defaultOpenMode,
               const CacheMode    cacheMode = defaultCacheMode);

    Connection(const Connection&) = delete;

    Connection(Connection&& connection) noexcept = default;

    virtual ~Connection();

    void close() noexcept;

    bool commit() noexcept;

    bool execute(const char* const query) noexcept;

    bool execute(const std::string& query) noexcept;

    std::string databaseName() const noexcept;

    bool isOpen() const noexcept;

    std::string lastError() const;

    int64_t lastInsertRowId() const noexcept;

    int lastResultCode() const noexcept;

    bool open();

    Statement prepare(const char* const query,
                      const int         length = -1) noexcept;

    Statement prepare(const std::string& query) noexcept;

    double readDouble(const std::string& query,
                      int*               resultCode = nullptr) noexcept;

    int64_t readInt64(const std::string& query,
                      int*               resultCode = nullptr) noexcept;

    std::string readString(const std::string& query,
                           int*               resultCode = nullptr);

    std::u16string readString16(const std::string& query,
                                int*               resultCode = nullptr);

    bool rollback() noexcept;

    void setDbName(const std::string& dbPath);

    bool transaction() noexcept;

    Connection& operator=(const Connection&) = delete;

    Connection& operator=(Connection&& connection) noexcept;

    static int setDefaultThreadMode(const ThreadMode value);

    static ThreadMode defaultThreadMode() noexcept;

    static int openedConnNumber() noexcept;

private:

    sqlite3* _db;

    std::string _dbName;
    std::string _openErrorMsg;

    OpenMode   _openMode;
    CacheMode  _cacheMode;

    int _lastResultCode;

    static std::mutex _mutex;
    static std::atomic_uint _openedConn;
    static std::atomic<ThreadMode> _libThreadMode;

    int getOpenFlags() const noexcept;

    int openInMemoryDb();

    int openRegularDb();

    int openTemporaryDb();

    int readValue(const std::string&                  query,
                  std::function<void (sqlite3_stmt*)> readLambda);

    static int configOptionFor(const ThreadMode value) noexcept;

    static int tryConfigThreadMode(const int option) noexcept;

};

#endif
