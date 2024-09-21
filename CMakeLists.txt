cmake_minimum_required(VERSION 3.29)
project(SimpleWebsocketClient)

set(CMAKE_CXX_STANDARD 23)

add_executable(SimpleWebsocketClient src/main.cpp
        src/WebsocketClient.cpp
        src/WebsocketClient.h)

find_package(Catch2 CONFIG REQUIRED)

add_executable(WebsocketClientTest test/WebsocketClientTest.cpp test/WebsocketMockServer.cpp)
target_link_libraries(WebsocketClientTest  PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
target_include_directories(WebsocketClientTest PRIVATE src test)