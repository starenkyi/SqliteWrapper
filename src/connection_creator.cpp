#include "../include/connection_creator.h"

#include "../include/create_conn_exception.h"

using Container = std::unordered_map<std::string, ConnectionConfig>;
using LockGuard = std::lock_guard<std::mutex>;


ConnectionCreator::ConnectionCreator(const ConnectionCreator& conn)
{
    // lock mutex
    LockGuard lock(conn._mutex);

    // copy content
    _configurations = conn._configurations;
}

ConnectionCreator::ConnectionCreator(ConnectionCreator&& conn)
{
    // lock mutex
    LockGuard lock(conn._mutex);

    // move content
    _configurations = std::move(conn._configurations);
}

ConnectionCreator::~ConnectionCreator() noexcept
{
    try {
        // lock mutex
        LockGuard lock(_mutex);

        // clear saved configs
        _configurations.clear();
    } catch (...) {}
}

bool ConnectionCreator::addConfig(const ConnectionConfig& value,
                                  const std::string&      name)
{
    // lock mutex
    LockGuard lock(_mutex);

    // try insert new config and return result
    return _configurations.emplace(name, value).second;
}

void ConnectionCreator::addOrReplaceConfig(const ConnectionConfig& value,
                                           const std::string&      name)
{
    // lock mutex
    LockGuard lock(_mutex);

    // insert new (or replace existing) config
    _configurations[name] = value;
}

void ConnectionCreator::clearConfigs()
{
    // lock mutex
    LockGuard lock(_mutex);

    // clear saved configs
    _configurations.clear();
}

std::vector<std::string> ConnectionCreator::configsArray() const
{
    // lock mutex
    LockGuard lock(_mutex);

    // reserve memory for array
    std::vector<std::string> result;
    result.reserve(_configurations.size());

    // read and save all configs names
    for (auto it = _configurations.cbegin();
         it != _configurations.cend(); ++it) {
        result.push_back(it->first);
    }

    // return array of configs names
    return result;
}

std::pair<ConnectionConfig, bool>
ConnectionCreator::configByName(const std::string& name) const noexcept
{
    std::pair<ConnectionConfig, bool> result;    // default result

    // lock mutex
    LockGuard lock(_mutex);

    // try find config with name 'name' (and assign to result, if any)
    Container::const_iterator it = _configurations.find(name);
    result.second = (it != _configurations.cend());
    if (result.second) {
        result.first = it->second;
    }

    // return found (or default) config with bool flag
    return result;
}

int ConnectionCreator::configsCount() const noexcept
{
    // lock mutex
    LockGuard lock(_mutex);

    // return number of saved configurations
    return _configurations.size();
}

bool ConnectionCreator::deleteConfig(const std::string& name)
{
    // lock mutex
    LockGuard lock(_mutex);

    return _configurations.erase(name) > 0;
}

bool ConnectionCreator::isConfigExists(const std::string& name) const noexcept
{
    // lock mutex
    LockGuard lock(_mutex);

    // check if config exist and return result
    return _configurations.find(name) != _configurations.cend();
}

Connection ConnectionCreator::newConnection(const std::string& configName) const
{
    // try find config with 'configName' (or throw if not exists)
    std::pair<ConnectionConfig, bool> conf = configByName(configName);
    if (!conf.second) {
        std::string errorMsg("Error: \'");
        throw CreateConnException(errorMsg.append(configName)
                                  .append("\' configuration not found!"));
    }

    std::string openErrorMsg;   // for error message in exception object

    Connection result(conf.first.databaseName(), conf.first.openMode(),
                      conf.first.cacheMode());

    // try open connection and throw on error
    if (!result.open()) {
        openErrorMsg = "Error opening database: ";
    // try create database schema
    } else if (!createSchema(result, conf.first.createSchemaScript())) {
        openErrorMsg = "Error creating database schema: ";
    // try configure connection
    } else if (!configureConnection(result,
                                    conf.first.configConnectionScript())) {
        openErrorMsg = "Error during connection configuration: ";
    }

    // throw if error occured (connection will close automatically)
    if (!openErrorMsg.empty()) {
        throw CreateConnException(openErrorMsg.append(result.lastError()));
    }

    // return created object
    return result;
}

bool ConnectionCreator::replaceConfig(const std::string&      name,
                                      const ConnectionConfig& newValue)
{
    // lock mutex
    LockGuard guard(_mutex);

    // try replace config and return result
    Container::iterator it = _configurations.find(name);
    if (it != _configurations.end()) {
        it->second = newValue;
        return true;
    } else {
        return false;
    }
}

bool
ConnectionCreator::configureConnection(Connection&        connection,
                                       const std::string& script) const noexcept
{
    return script.empty() || connection.execute(script);
}

bool ConnectionCreator::createSchema(Connection&        connection,
                                     const std::string& script) const noexcept
{
    // create db schema, if not exists
    return script.empty()
            || connection.readInt64("select count(*) from sqlite_master")
            || connection.execute(script);
}
