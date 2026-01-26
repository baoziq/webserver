cmake_minimum_required(VERSION 3.10)

project(webserver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)
add_library(Mylib STATIC
    src/buffer.cc
    src/epoller.cc
    src/connection.cc
)

add_executable(webserver  src/main.cc)

target_link_libraries(webserver PRIVATE Mylib)
