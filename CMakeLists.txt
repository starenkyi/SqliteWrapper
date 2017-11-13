cmake_minimum_required(VERSION 2.8)

project(SqliteWrapper)

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/bin)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/bin)

SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/lib)

enable_testing()

include_directories(include)
add_subdirectory(src)
add_subdirectory(test)
