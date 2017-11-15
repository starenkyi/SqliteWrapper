#ifndef CONNECTION_CREATOR_H
#define CONNECTION_CREATOR_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

#include "connection.h"
#include "connection_config.h"


class ConnectionCreator
{

public:

    ConnectionCreator() = default;

    ConnectionCreator(const ConnectionCreator& conn);

    ConnectionCreator(ConnectionCreator&& conn);

    ~ConnectionCreator() noexcept;

    bool addConfig(const ConnectionConfig& value,
                   const std::string&      name);

    void addOrReplaceConfig(const ConnectionConfig& value,
                            const std::string&      name);

    void clearConfigs();

    std::vector<std::string> configsArray() const;

    std::pair<ConnectionConfig, bool>
    configByName(const std::string& name) const noexcept;

    int configsCount() const noexcept;

    bool deleteConfig(const std::string& name);

    bool isConfigExists(const std::string& name) const noexcept;

    Connection newConnection(const std::string& configName) const;

    bool replaceConfig(const std::string&      name,
                       const ConnectionConfig& newValue);

    ConnectionCreator& operator=(const ConnectionCreator& conn) = delete;

    ConnectionCreator& operator=(ConnectionCreator&& conn) = delete;

private:

    mutable std::mutex _mutex;

    std::unordered_map<std::string, ConnectionConfig> _configurations;

    bool configureConnection(Connection&        connection,
                             const std::string& script) const noexcept;

    bool createSchema(Connection&        connection,
                      const std::string& script) const noexcept;

};

#endif
