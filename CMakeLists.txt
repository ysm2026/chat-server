cmake_minimum_required(VERSION 3.10)
project(ChatServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chat_server
    src/main.cpp
    src/Server.cpp
)

set_target_properties(chat_server PROPERTIES
    LINK_FLAGS "-static-libgcc -static-libstdc++ -static"
)

target_include_directories(chat_server PRIVATE src)

add_executable(chat_client
    src/main_client.cpp
    src/Client.cpp
)

set_target_properties(chat_client PROPERTIES
    LINK_FLAGS "-static-libgcc -static-libstdc++ -static"
)

target_include_directories(chat_client PRIVATE src)

if(WIN32)
    target_link_libraries(chat_server PRIVATE ws2_32)
    target_link_libraries(chat_client PRIVATE ws2_32)
endif()
