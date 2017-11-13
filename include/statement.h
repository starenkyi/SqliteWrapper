#ifndef DB_STATEMENT_H
#define DB_STATEMENT_H

#include <cstdint>
#include <string>

struct sqlite3_stmt;
struct sqlite3;


class Statement
{

public:

    enum Type : int {
        Undefined = 0,
        Select,
        NonSelect
    };

    Statement() noexcept;

    Statement(const Statement& statement) noexcept = delete;

    Statement(Statement&& statement) noexcept;

    explicit Statement(sqlite3_stmt* statement) noexcept;

    ~Statement() noexcept;

    bool bindBlob(const int         index,
                  const void* const value,
                  const int         bytes) const noexcept;

    bool bindBlobCopy(const int         index,
                      const void* const value,
                      const int         bytes) const noexcept;

    bool bindBool(const int  index,
                  const bool value) const noexcept;

    bool bindCStr(const int         index,
                  const char* const value,
                  const int         bytes = -1) const noexcept;

    bool bindCStr16(const int         index,
                    const void* const value,
                    const int         bytes = -1) const noexcept;

    bool bindCStrCopy(const int         index,
                      const char* const value,
                      const int         bytes = -1) const noexcept;

    bool bindCStr16Copy(const int         index,
                        const void* const value,
                        const int         bytes = -1) const noexcept;

    bool bindDouble(const int    index,
                    const double value) const noexcept;

    bool bindInt(const int index,
                 const int value) const noexcept;

    bool bindInt64(const int     index,
                   const int64_t value) const noexcept;

    bool bindNull(const int index) const noexcept;

    bool bindString(const int          index,
                    const std::string& value) const noexcept;

    bool bindString16(const int             index,
                      const std::u16string& value) const noexcept;

    bool bindStringCopy(const int          index,
                        const std::string& value) const noexcept;

    bool bindString16Copy(const int            index,
                          const std::u16string& value) const noexcept;

    int32_t byteLength(const int index) const noexcept;

    int32_t byteLength16(const int index) const noexcept;

    void clear() noexcept;

    void clearBindings() const noexcept;

    int columnCount() const noexcept;

    int columnType(const int index) const noexcept;

    bool execute() const noexcept;

    std::string expandedQuery() const;

    const unsigned char* getBlob(const int index,
                                 int&      bytes) const noexcept;

    unsigned char* getBlobCopy(const int index,
                               int&      bytes) const;

    bool getBool(const int index) const noexcept;

    const char* getCStr(const int index,
                        int&      bytes) const noexcept;

    const char16_t* getCStr16(const int index,
                              int&      bytes) const noexcept;

    char* getCStrCopy(const int index,
                      int&      bytes) const;

    char16_t* getCStr16Copy(const int index,
                            int&      bytes) const;

    double getDouble(const int index) const noexcept;

    float getFloat(const int index) const noexcept;

    int getInt(const int index) const noexcept;

    int64_t getInt64(const int index) const noexcept;

    std::string getString(const int index) const;

    std::u16string getString16(const int index) const;

    bool isNull(const int index) const noexcept;

    bool isValid() const noexcept;

    std::string lastError() const;

    std::u16string lastError16() const;

    int lastErrorCode() const noexcept;

    int64_t lastInsertRowId() const noexcept;

    bool next() const noexcept;

    std::string query() const;

    Type type() const noexcept;

    Statement &operator=(const Statement& statement) noexcept = delete;

    Statement &operator=(Statement&& statement) noexcept;

protected:

    void reset() noexcept;

private:

    sqlite3_stmt* _statement;
    sqlite3* _db;
    int _columnCount;
    Type _type;

};

#endif
