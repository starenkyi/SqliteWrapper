#include <cassert>
#include <iostream>
#include <string>

#include "../include/connection.h"


static const std::string script("PRAGMA foreign_keys = off;"
                                "BEGIN TRANSACTION; "
                                "CREATE TABLE Person (id INTEGER NOT NULL "
                                "PRIMARY KEY, name TEXT NOT NULL, "
                                "weight DOUBLE);"
                                "COMMIT TRANSACTION;"
                                "PRAGMA foreign_keys = on;");
static const std::string fileName("test.db");


std::string testConnection(Connection&& conn) {
    // test isOpen
    assert(conn.isOpen());

    // test executing query (create schema)
    assert(conn.execute(script));

    // test insert within translaction
    assert(conn.transaction());
    assert(conn.execute(std::string("INSERT into Person (id, name, weight) "
                                    "VALUES (1, \'mike\', 0.0)")));
    assert(conn.lastInsertRowId() == 1);
    assert(conn.execute(std::string("INSERT into Person (id, name, weight) "
                                    "VALUES (2, \'chris\', 70.9)")));
    assert(conn.lastInsertRowId() == 2);
    assert(conn.commit());

    // test update without transaction
    assert(conn.execute(std::string("UPDATE Person SET name = \'kate\' "
                                    "WHERE id = 2")));

    int result;

    // test select int64
    int iVal = conn.readInt64("SELECT id FROM Person WHERE name = \'mike\'",
                              &result);
    assert(iVal == 1 && result == Connection::ReadSuccess);

    // test select double
    double dVal = conn.readDouble("SELECT weight FROM Person WHERE id = 1",
                                  &result);
    assert(dVal == 0.0 && result == Connection::ReadSuccess);

    // test select string
    std::string sVal = conn.readString("SELECT name FROM Person WHERE id = 2",
                                       &result);
    assert(sVal == "kate" && result == Connection::ReadSuccess);

    // test unvalid queries execution
    assert(!conn.execute("INSERT into Person (id, name, weight) "));
    iVal = conn.readInt64("SELECT ids FROM Person WHERE name = \'mike\'",
                          &result);
    assert(iVal == 0 && result > 0);

    // test empty read
    dVal = conn.readDouble("SELECT weight FROM Person WHERE id = 5",
                           &result);
    assert(dVal == 0.0 && result == Connection::EmptyData);

    // test read non-select query
    iVal = conn.readInt64("INSERT into Person (id, name, weight) "
                          "VALUES (4, \'tom\', NULL)",
                          &result);
    assert(iVal == 0 && result == Connection::NoData);

    // test read NULL data
    dVal = conn.readDouble("SELECT weight FROM Person WHERE id = 4",
                           &result);
    assert(dVal == 0.0/* && result == Connection::NullValue*/);

    // close connection
    conn.close();
    assert(!conn.isOpen());

    return std::string("OK");
}

void testRWConnection() {
    std::string dbName("rw_test.db");

    // create database
    Connection create(dbName);
    assert(create.open());
    assert(create.execute(script));
    assert(create.execute(std::string("INSERT into Person (id, name, weight) "
                                      "VALUES (1, \'mike\', 0.0)")));
    create.close();

    Connection conn(dbName, Connection::OpenMode::ReadWrite);
    assert(conn.open());

    // test insert operation
    assert(conn.execute(std::string("INSERT into Person (id, name, weight) "
                                    "VALUES (8, \'john\', 80.1)")));
    //std::cout << "\nError: " << conn.lastError() << std::endl;
    //std::cout << "Eroro code: " << conn.lastResultCode() << std::endl;
    assert(!conn.lastResultCode());

    // test select
    int result;
    int iVal = conn.readInt64("SELECT id FROM Person WHERE name = \'mike\'",
                              &result);
    assert(iVal == 1 && result == Connection::ReadSuccess);

    // close connection
    conn.close();

    // delete file
    std::remove(dbName.c_str());
}

void testROConnection() {
    Connection conn(fileName, Connection::OpenMode::ReadOnly);
    assert(conn.open());

    // test insert operation (not allowed in RO-mode)
    assert(!conn.execute(std::string("INSERT into Person (id, name, weight) "
                                     "VALUES (10, \'john\', 80.0)")));

    // test select
    int result;
    int iVal = conn.readInt64("SELECT id FROM Person WHERE name = \'mike\'",
                              &result);
    assert(iVal == 1 && result == Connection::ReadSuccess);

    // close connection
    conn.close();
}

std::string testRegularConnection() {
    Connection conn(fileName, Connection::OpenMode::ReadWriteCreate);
    assert(conn.open());

    // test read-write-create connection
    std::string res = testConnection(std::move(conn));

    // test read-write connection
    testRWConnection();

    // test read-only connection
    testROConnection();

    // delete file of created db
    std::remove(fileName.c_str());

    // return result
    return res;
}

std::string testTempConnection() {
    Connection conn(Connection::OpenMode::Temporary);
    assert(conn.open());

    // test temporary db connection and return result
    return testConnection(std::move(conn));
}

std::string testMemoryConnection() {
    // test open in-memory db connection without name
    Connection conn1(Connection::OpenMode::InMemory);
    assert(conn1.open());

    Connection conn2("mem1", Connection::OpenMode::InMemory);
    assert(conn2.open());

    // test in-memory db connection within name and return result
    return testConnection(std::move(conn2));
}

int main() {

    // test change thread mode
    assert(Connection::setDefaultThreadMode
            (Connection::ThreadMode::SingleThread) == Connection::Ok);
    assert(Connection::defaultThreadMode()
            == Connection::ThreadMode::SingleThread);

    std::cout << "Test create and use regular connection: "
              << testRegularConnection() << std::endl;
    std::cout << "Test create and use connection to temp database: "
              << testTempConnection() << std::endl;
    std::cout << "Test create and use connection to in-memory database: "
              << testMemoryConnection() << std::endl;

    // test change thread mode
    std::cout << "Test change thread mode: OK" << std::endl;
    assert(Connection::setDefaultThreadMode
            (Connection::ThreadMode::Serialized) == Connection::Ok);
    assert(Connection::defaultThreadMode()
        == Connection::ThreadMode::Serialized);

    return 0;
}
