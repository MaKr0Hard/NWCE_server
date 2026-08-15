cmake_minimum_required(VERSION 3.10)

project(nwce)

add_executable(nwce main.cpp)

install(TARGETS nwce RUNTIME DESTINATION bin)
