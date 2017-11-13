#ifndef CREATE_CONN_EXCEPTION_H
#define CREATE_CONN_EXCEPTION_H

#include <stdexcept>
#include <string>


class CreateConnException : public std::runtime_error
{

public:

    explicit CreateConnException(const std::string& message = std::string());

    explicit CreateConnException(const char* message);

    virtual ~CreateConnException() = default;

    std::string errorMessage() const;

};

#endif
