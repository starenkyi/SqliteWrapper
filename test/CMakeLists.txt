cmake_minimum_required(VERSION 2.8)

project(SqliteWrapper-Test)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-Wall -O0)

add_executable(test_connection test_connection.cpp)
target_link_libraries(test_connection SqliteWrapper)
add_test(NAME test_connection COMMAND test_connection)

add_executable(test_statement test_statement.cpp)
target_link_libraries(test_statement SqliteWrapper)
add_test(NAME test_statement COMMAND test_statement)

add_executable(test_connection_creator test_connection_creator.cpp)
target_link_libraries(test_connection_creator SqliteWrapper)
add_test(NAME test_connection_creator COMMAND test_connection_creator)
