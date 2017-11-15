#include <cassert>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "../include/connection.h"
#include "../include/connection_config.h"
#include "../include/connection_creator.h"
#include "../include/create_conn_exception.h"


static const std::string script("PRAGMA foreign_keys = off;"
                                "BEGIN TRANSACTION; "
                                "CREATE TABLE Person (id INTEGER NOT NULL "
                                "PRIMARY KEY, name TEXT NOT NULL);"
                                "COMMIT TRANSACTION;"
                                "PRAGMA foreign_keys = on;");
static const std::string fileName("test.db");


std::string testRemoveReplaceConfig() {
    // create configurations
    ConnectionConfig config;
    config.setDatabaseName(std::string(fileName));
    config.setCreateSchemaScript(script);

    ConnectionConfig config2;
    config.setDatabaseName(std::string("test2.db"));
    config.setCreateSchemaScript(script);

    ConnectionCreator creator;

    // test add config
    assert(creator.addConfig(config, "default"));
    std::pair<ConnectionConfig, bool> conf = creator.configByName("default");
    assert(conf.second && conf.first.equal(config));

    // test if configs list is not empty
    assert(creator.configsCount() == 1);

    // test if config exists
    assert(creator.isConfigExists("default"));

    // test add config with existing name
    assert(!creator.addConfig(config, "default"));

    // test addReplace config
    creator.addOrReplaceConfig(config2, "default");
    conf = creator.configByName("default");
    assert(conf.second && conf.first.equal(config2));

    // test replace config
    assert(creator.replaceConfig("default", config));
    conf = creator.configByName("default");
    assert(conf.second && conf.first.equal(config));

    // test replace not existing config
    assert(!creator.replaceConfig("default1", config));

    // test add config with another name
    assert(creator.addConfig(config, "other"));

    // test configs count
    assert(creator.configsCount() == 2);

    // test search not existing config
    conf = creator.configByName("def");
    assert(!conf.second && !conf.first.equal(config));

    // test get configs name array
    auto list = creator.configsArray();
    assert(list.size() == 2);

    // test configs name
    std::sort(list.begin(), list.end());
    assert(list.at(0) == "default" && list.at(1) == "other");

    // test delete config
    assert(creator.deleteConfig("other") && creator.configsCount() == 1);

    // test delete not existing config
    assert(!creator.deleteConfig("other"));

    // test clear configs
    creator.clearConfigs();
    assert(creator.configsCount() == 0);

    return std::string("OK");
}

std::string testOpenConn() {
    // create configuration
    ConnectionConfig config;
    config.setOpenMode(Connection::OpenMode::ReadWriteCreate);
    config.setCacheMode(Connection::CacheMode::Private);
    config.setDatabaseName(std::string(fileName));
    config.setCreateSchemaScript(script);

    // test add connection configuration
    ConnectionCreator creator;
    assert(creator.addConfig(config, "default"));

    // test open connection configuration
    Connection conn = creator.newConnection("default");

    // delete created file
    std::remove(fileName.c_str());

    return std::string("OK");
}

std::string testOpenInvalidConn() {
    // create configuration
    ConnectionConfig config;
    config.setOpenMode(Connection::OpenMode::ReadOnly);
    config.setCacheMode(Connection::CacheMode::Private);
    config.setDatabaseName(std::string(fileName));
    config.setCreateSchemaScript(script);

    // test open connection by configuration that not exists
    ConnectionCreator creator;
    try {
        Connection conn = creator.newConnection("default");
        throw std::runtime_error("Connection config must not exists!");
    } catch (CreateConnException e) {
    }

    // test open connection by ivalid configuration
    // (sqlite can't open read-only database if file not exists)
    creator.addConfig(config, "invalid");
    try {
        Connection conn = creator.newConnection("invalid");
        throw std::runtime_error("Connection config is ivalid and can't use "
                                 "for open connection!");
    } catch (CreateConnException e) {
    }

    return std::string("OK");
}

int main() {

    std::cout << "Add, replace, remove connection config: "
              << testRemoveReplaceConfig() << std::endl;
    std::cout << "Open valid connection: " << testOpenConn() << std::endl;
    std::cout << "Open connection with invalid config (or config name): "
              << testOpenInvalidConn() << std::endl;
    return 0;
}
