cmake_minimum_required(VERSION 2.8)

project(SqliteWrapper)

enable_testing()

include_directories(include)
add_subdirectory(src)
add_subdirectory(test)
