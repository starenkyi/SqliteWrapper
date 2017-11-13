#include "../include/create_conn_exception.h"


CreateConnException::CreateConnException(const std::string& message)
    : std::runtime_error(message)
{}

CreateConnException::CreateConnException(const char* message)
    : std::runtime_error(message ? message : "")
{}

std::string CreateConnException::errorMessage() const
{
    return std::string(what());
}
