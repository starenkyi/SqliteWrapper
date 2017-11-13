#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

#include "../include/connection.h"
#include "../include/statement.h"


std::string testUtf8() {
    static const std::string fileName("test_utf8.db");
    static const char* text1 = "text1";
    static const char* text2 = "text2";
    static const char* text3 = "text3";
    static const char blob1[5] = { 'b', 'l', 'o', 'b', '1' };
    static const char* blob2 = "blob2";

    Connection conn(fileName);

    // open connection and create db schema
    assert(conn.open());
    assert(conn.isOpen());
    assert(conn.execute("PRAGMA encoding=\"UTF-8\";"
                        "PRAGMA foreign_keys = off; BEGIN TRANSACTION;"
                        "CREATE TABLE IF NOT EXISTS Person (id INTEGER NOT "
                        "NULL PRIMARY KEY, name TEXT, age INT (4), weight "
                        "DOUBLE, present BOOLEAN, binData BLOB);"
                        "COMMIT TRANSACTION; PRAGMA foreign_keys=on;"));

    // begin transaction and prepare statement
    assert(conn.transaction());
    Statement s = conn.prepare("INSERT OR REPLACE INTO Person(id,name,age,"
                               "weight,present,binData) VALUES(?,?,?,?,?,?)");

    // check statement
    assert(s.isValid());
    assert(s.type() == Statement::NonSelect);

    // test insert data
    assert(s.bindInt64(1, 1));
    assert(s.bindCStr(2, text1));
    assert(s.bindDouble(4, 90.3));
    assert(s.bindBool(5, true));
    assert(s.bindBlob(6, blob1, 5));
    assert(s.execute());
    assert(s.lastInsertRowId() == 1);

    assert(s.bindInt64(1, 2));
    assert(s.bindNull(2));
    assert(s.bindInt(3, 20));
    assert(s.bindNull(4));
    assert(s.bindBool(5, false));
    assert(s.bindBlobCopy(6, blob2, 5));
    assert(s.execute());

    assert(s.bindInt64(1, 3));
    assert(s.bindStringCopy(2, std::string(text2)));
    assert(s.execute());

    assert(s.bindInt64(1, 4));
    assert(s.bindCStrCopy(2, text2));
    assert(s.execute());

    assert(s.bindInt64(1, 5));
    std::string tStr(text3);
    assert(s.bindString(2, tStr));
    assert(s.execute());

    // commit changes
    assert(conn.commit());

    // prepare read statement and check it
    std::string query("SELECT * FROM Person ORDER BY id ASC");
    s = conn.prepare(query);
    assert(s.isValid());
    assert(s.type() == Statement::Select);
    assert(s.columnCount() == 6);
    assert(s.query() == query);

    // test read data
    int bytes;
    char* temp;
    assert(conn.readInt64("SELECT count(*) FROM Person") == 5);

    assert(s.next());
    assert(s.getInt(0) == 1);
    assert(s.getInt64(0) == 1);
    assert(s.isNull(2));
    assert(s.getInt(2) == 0);
    assert(s.getInt64(2) == 0);
    assert(s.getDouble(3) == 90.3);
    assert(s.getBool(4) == true);

    assert(!strcmp(s.getCStr(1, bytes), text1));
    assert(s.getString(1) == text1);
    temp = s.getCStrCopy(1, bytes);
    assert(!strcmp(temp, text1));
    delete [] temp;
    assert(s.byteLength(1) == static_cast<int>(strlen(text1)));

    const char* b1 = reinterpret_cast<const char*>(s.getBlob(5, bytes));
    assert(!strncmp(b1, blob1, bytes));
    b1 = reinterpret_cast<const char*>(s.getBlobCopy(5, bytes));
    assert(!strncmp(b1, blob1, bytes));
    delete [] b1;
    assert(s.byteLength(5) == static_cast<int>(strlen(text1))
           && s.byteLength(5) == 5);

    assert(s.next());
    assert(s.getInt64(0) == 2);
    assert(s.getInt(2) == 20);
    assert(s.isNull(3));
    assert(s.getDouble(3) == 0.0);
    assert(s.getBool(4) == false);

    assert(s.isNull(1));
    assert(s.getCStr(1, bytes) == nullptr);
    assert(s.getCStr(1, bytes) == NULL);
    assert(bytes == 0);
    assert(s.getCStrCopy(1, bytes) == nullptr);
    assert(s.getCStrCopy(1, bytes) == NULL);
    assert(bytes == 0);
    assert(s.getString(1).empty());
    assert(s.byteLength(1) == 0);

    assert(s.next());
    assert(s.next());
    assert(s.next());
    assert(!s.next());
    s.clear();
    assert(!s.isValid());

    // close connection and remove db file
    conn.close();
    assert(!conn.isOpen());
    std::remove(fileName.c_str());

    return std::string("OK");
}

std::string testUtf16() {
    static const std::string fileName("test_utf16.db");
    static const char16_t* text1 = u"text1";
    static const char16_t* text2 = u"text2";
    static const char16_t* text3 = u"text3";
    static const char16_t* text4 = u"text4";

    Connection conn(fileName);

    // open connection and create db schema
    assert(conn.open());
    assert(conn.isOpen());
    assert(conn.execute("PRAGMA encoding=\"UTF-16\";"
                        "PRAGMA foreign_keys = off; BEGIN TRANSACTION;"
                        "CREATE TABLE IF NOT EXISTS Person (id INTEGER NOT "
                        "NULL PRIMARY KEY, name TEXT);"
                        "COMMIT TRANSACTION; PRAGMA foreign_keys=on;"));

    // begin transaction and prepare statement
    assert(conn.transaction());
    Statement s = conn.prepare("INSERT OR IGNORE INTO Person(id,name) "
                               "VALUES(?,?)");

    // check statement
    assert(s.isValid());
    assert(s.type() == Statement::NonSelect);

    // test insert data
    assert(s.bindInt64(1, 1));
    assert(s.bindCStr16(2, text1));
    assert(s.execute());

    assert(s.bindInt64(1, 2));
    assert(s.bindNull(2));
    assert(s.execute());

    assert(s.bindInt64(1, 3));
    assert(s.bindCStr16Copy(2, text2));
    assert(s.execute());

    assert(s.bindInt64(1, 4));
    std::u16string tStr(text3);
    assert(s.bindString16(2, tStr));
    assert(s.execute());

    assert(s.bindInt64(1, 5));
    assert(s.bindString16Copy(2, std::u16string(text4)));
    assert(s.execute());

    // commit changes
    assert(conn.commit());

    // prepare read statement and check it
    std::string query("SELECT * FROM Person ORDER BY id ASC");
    s = conn.prepare(query);
    assert(s.isValid());
    assert(s.type() == Statement::Select);
    assert(s.columnCount() == 2);

    // test read data
    int bytes;
    const char16_t* temp;
    assert(conn.readInt64("SELECT count(*) FROM Person") == 5);

    assert(s.next());
    assert(s.getInt64(0) == 1);
    assert(!s.isNull(1));

    temp = s.getCStr16(1, bytes);
    assert(!memcmp(temp, text1, bytes));
    assert(s.getString16(1) == text1);
    temp = s.getCStr16Copy(1, bytes);
    assert(!memcmp(temp, text1, bytes));
    delete [] temp;

    std::u16string st(text1);
    assert(s.byteLength(1) == static_cast<int>(st.size()));

    assert(s.next());
    assert(s.getInt64(0) == 2);
    assert(s.isNull(1));
    assert(s.getCStr16(1, bytes) == nullptr);
    assert(s.getCStr16(1, bytes) == NULL);
    assert(bytes == 0);
    assert(s.getCStr16Copy(1, bytes) == nullptr);
    assert(bytes == 0);
    assert(s.getString16(1).empty());
    assert(s.byteLength(1) == 0);

    assert(s.next());
    assert(s.next());
    assert(s.next());
    assert(!s.next());
    s.clear();
    assert(!s.isValid());

    // close connection and remove db file
    conn.close();
    assert(!conn.isOpen());
    std::remove(fileName.c_str());

    return std::string("OK");
}


int main() {

    std::cout << "Test statement on UTF-8 encoded database: "
              << testUtf8() << std::endl;
    std::cout << "Test statement on UTF-16 encoded database: "
              << testUtf16() << std::endl;

    return 0;
}
