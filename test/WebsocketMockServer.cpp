#include "WebsocketMockServer.h"

#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <chrono>
#include <cstring>

WebsocketMockServer::WebsocketMockServer(int port) {
    this->socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->socketFd == -1) {
        throw std::runtime_error("Failed to create socket");
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(this->socketFd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(this->socketFd, 3) == -1) {
        throw std::runtime_error("Failed to listen on socket");
    }
}

void WebsocketMockServer::start() {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int new_socket = accept(this->socketFd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket == -1) {
        throw std::runtime_error("Failed to accept connection");
    }

    char buffer[1024] = {0};
    int valread = read(new_socket, buffer, 1024);
    if (valread == -1) {
        throw std::runtime_error("Failed to read from socket");
    }

    std::string response = "HTTP/1.1 101 Switching Protocols\r\n"
                           "Upgrade: websocket\r\n"
                           "Connection: Upgrade\r\n"
                           "Sec-WebSocket-Accept: HSmrc0sMlYUkAGmm5OPpG2HaGWk=\r\n\r\n";
    send(new_socket, response.c_str(), response.size(), 0);

    int currentTime();
    int milis = currentTime();

    for (auto const& [delay, message] : messageMap) {
        if (currentTime() - milis >= delay) {
            send(new_socket, message.c_str(), message.size(), 0);
        }
    }
}

void WebsocketMockServer::stop() {
    close(this->socketFd);
}

void WebsocketMockServer::emitMessage(int delay, char* message, int opcode) {
      messageMap[delay] = std::string(1, static_cast<char>(opcode)) + std::string(message, strlen(message));
}

int currentTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
       std::chrono::system_clock::now().time_since_epoch()
    ).count();
}