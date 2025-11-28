cmake_minimum_required(VERSION 3.10)
project(pg_rest_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX libpqxx)

include_directories(${PQXX_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/third_party)
link_directories(${PQXX_LIBRARY_DIRS})

add_executable(server src/server.cpp)
target_link_libraries(server ${PQXX_LIBRARIES} pthread)

add_executable(client src/client.cpp)
target_link_libraries(client pthread)
