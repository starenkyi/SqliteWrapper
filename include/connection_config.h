#ifndef CONNECTION_CONFIG_H
#define CONNECTION_CONFIG_H

#include <string>

#include "connection.h"


class ConnectionConfig
{

public:

    ConnectionConfig();

    ConnectionConfig(const ConnectionConfig& config) = default;

    ConnectionConfig(ConnectionConfig&& config) noexcept = default;

    ~ConnectionConfig() noexcept = default;

    std::string databaseName() const;

    Connection::CacheMode cacheMode() const noexcept;

    std::string configConnectionScript() const;

    std::string createSchemaScript() const;

    bool equal(const ConnectionConfig& config) const noexcept;

    Connection::OpenMode openMode() const noexcept;

    void setDatabaseName(const std::string& databaseName);

    void setCacheMode(const Connection::CacheMode value) noexcept;

    void setConfigConnectionScript(const std::string& script);

    void setCreateSchemaScript(const std::string& script);

    void setOpenMode(const Connection::OpenMode value) noexcept;

    ConnectionConfig& operator=(const ConnectionConfig& config) = default;

    ConnectionConfig& operator=(ConnectionConfig&& config) noexcept = default;

private:

    std::string _databaseName;
    std::string _createSchemaScript;
    std::string _configConnectionScript;

    Connection::CacheMode _cacheMode;
    Connection::OpenMode  _openMode;

};

#endif
