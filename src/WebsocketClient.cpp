#include "WebsocketClient.h"

#include <strings.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>

void WebsocketClient::connect(const std::string &protocol, const std::string &hostname, int *port, std::string *path) {
    socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketFd < 0) {
        perror("socket");
        return;
    }

    struct hostent *server = gethostbyname(hostname.c_str());
    if (server == NULL) {
        perror("gethostbyname");
        return;
    }

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(*port);
    bcopy((char *) server->h_addr, (char *) &server_addr.sin_addr.s_addr, server->h_length);

    if (::connect(socketFd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return;
    }

    std::string handshake = "GET / HTTP/1.1\r\n"
                            "Host: " + createUrl(protocol, hostname, port, path) + "\r\n"
                            "Upgrade: websocket\r\n"
                            "Connection: Upgrade\r\n"
                            "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
                            "Sec-WebSocket-Version: 13\r\n\r\n";

    if (send(socketFd, handshake.c_str(), handshake.size(), 0) < 0) {
        perror("send");
        return;
    }

    char response[1024];
    int response_length = recv(socketFd, response, sizeof(response), 0);
    if (response_length < 0) {
        perror("recv");
        return;
    }

    std::string response_str(response, response_length);
    if (response_str.find("101 Switching Protocols") == std::string::npos) {
        std::cerr << "Handshake failed" << std::endl;
        return;
    }

    int flags = fcntl(socketFd, F_GETFL, 0);
    fcntl(socketFd, F_SETFL, flags | O_NONBLOCK);

    std::cout << "Handshake successful" << std::endl;
}

void WebsocketClient::setOnMessageCallback(std::function<void(char *, int, int)> callback) {
    this->callback = callback;
}

void WebsocketClient::createReadLoop() {
    messageThread = std::thread([this]() {
        int responseLength = 0;
        char response[responseMaxLength];
        int current = 0;
        while (true) {
            responseLength = recv(socketFd, response + current, responseMaxLength - current, 0);

            if (responseLength > 0)
                current += responseLength;

            while (current >= 2) {
                unsigned char opcode = response[0] & 0x0F;
                unsigned int payloadLength = response[1] & 0x7F;
                int headerLength = 2;

                if (payloadLength == 126 && current >= 4) {
                    payloadLength = (response[2] << 8) + response[3];
                    headerLength = 4;
                } else if (payloadLength == 127 && current >= 10) {
                    payloadLength = ((uint64_t)response[2] << 56) + ((uint64_t)response[3] << 48) +
                                    ((uint64_t)response[4] << 40) + ((uint64_t)response[5] << 32) +
                                    ((uint64_t)response[6] << 24) + ((uint64_t)response[7] << 16) +
                                    ((uint64_t)response[8] << 8) + response[9];
                    headerLength = 10;
                }

                if (current < headerLength + payloadLength) {
                    break;
                }

                if (opcode == 1 || opcode == 2) {
                    this->callback(response, headerLength, payloadLength);
                } else if (opcode == 8) {
                    std::cout << "Connection closed" << std::endl;
                    return;
                }

                current -= (headerLength + payloadLength);
            }
        }
    });


    messageThread.join();
}

void WebsocketClient::start() {
    createReadLoop();
}

void WebsocketClient::stop() {
    messageThread.detach();
}

std::string WebsocketClient::createUrl(std::string protocol, std::string hostname, int *port, std::string *path) {
    return protocol + "://" + hostname + (port == nullptr ? "" : ":" + std::to_string(*port)) + (
               path == nullptr ? "" : ("/" + *path));
}
