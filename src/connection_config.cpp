#include "../include/connection_config.h"


ConnectionConfig::ConnectionConfig()
    : _cacheMode(Connection::defaultCacheMode),
      _openMode(Connection::defaultOpenMode)
{}

std::string ConnectionConfig::databaseName() const
{
    return _databaseName;
}

Connection::CacheMode ConnectionConfig::cacheMode() const noexcept
{
    return _cacheMode;
}

std::string ConnectionConfig::configConnectionScript() const
{
    return _configConnectionScript;
}

std::string ConnectionConfig::createSchemaScript() const
{
    return _createSchemaScript;
}

bool ConnectionConfig::equal(const ConnectionConfig& config) const noexcept
{
    return !_databaseName.compare(config.databaseName())
            && _cacheMode == config.cacheMode()
            && _openMode == config.openMode()
            && !_createSchemaScript.compare(config.createSchemaScript())
            && !_configConnectionScript.compare(
                config.configConnectionScript());
}

Connection::OpenMode ConnectionConfig::openMode() const noexcept
{
    return _openMode;
}

void ConnectionConfig::setDatabaseName(const std::string& databaseName)
{
    _databaseName = databaseName;
}

void ConnectionConfig::setCacheMode(const Connection::CacheMode value) noexcept
{
    _cacheMode = value;
}

void ConnectionConfig::setConfigConnectionScript(const std::string& script)
{
    _configConnectionScript = script;
}

void ConnectionConfig::setCreateSchemaScript(const std::string& script)
{
    _createSchemaScript = script;
}

void ConnectionConfig::setOpenMode(const Connection::OpenMode value) noexcept
{
    _openMode = value;
}

