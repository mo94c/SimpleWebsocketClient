#include <iostream>
#include "WebsocketClient.h"

int main() {
    WebsocketClient client;
    int port = 8080;
    client.connect("ws", "localhost", &port, nullptr);
    client.setOnMessageCallback([](const char* buffer, int from, int length) {
        std::cout << "Received message: " << std::string(buffer + from, length) << std::endl;
    });
    client.start();

    std::cin >> port;

    return 0;
}
