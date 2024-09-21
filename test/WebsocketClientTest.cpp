#include <catch2/catch_test_macros.hpp>

#include "../src/WebsocketClient.h"
#include "WebsocketMockServer.h"
#include <iostream>

TEST_CASE("WebsocketClient") {
    int port = 7357;
    WebsocketMockServer server(port);

    server.emitMessage(500, "Hello", 1);
    server.start();

    WebsocketClient client;
    client.connect("ws", "localhost", &port, nullptr);
    client.setOnMessageCallback([](const char* buffer, int from, int length) {
        std::cout << "Received message: " << std::string(buffer + from, length) << std::endl;
    });
    client.start();
}