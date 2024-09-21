#include <map>
#include <string>

class WebsocketMockServer {
int socketFd;
std::map<int, std::string> messageMap;
public:
    WebsocketMockServer(int port);
    void start();
    void stop();
    void emitMessage(int delay, char* message, int opcode);
};