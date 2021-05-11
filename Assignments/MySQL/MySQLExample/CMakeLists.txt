
cmake_minimum_required(VERSION 3.8)
project("SQL_test")
include_directories(/usr/include/mysql)
add_executable(test "test.cpp")
target_link_libraries(test mysqlclient)