cmake_minimum_required(VERSION 3.0)

project(USB-Watcher)

add_executable(${PROJECT_NAME} USB-Watcher.c)

set (CMAKE_C_FLAGS "-g -Wall")
