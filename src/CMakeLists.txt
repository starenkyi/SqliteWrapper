cmake_minimum_required(VERSION 2.8)

project(SqliteWrapper)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads)

set(SOURCE_LIB sqlite3.c statement.cpp connection.cpp connection_config.cpp create_conn_exception.cpp connection_creator.cpp)

add_definitions(-Wall -O2)
add_library(${PROJECT_NAME} STATIC ${SOURCE_LIB})
target_link_libraries(${PROJECT_NAME} Threads::Threads ${CMAKE_DL_LIBS})
