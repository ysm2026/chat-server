cmake_minimum_required(VERSION 3.10)
project(ChatServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chat_server
    src/main.cpp
    src/Server.cpp
)

target_include_directories(chat_server PRIVATE src)

if(WIN32)
    target_link_libraries(chat_server PRIVATE ws2_32)
endif()
