#include <string>
#include <thread>
#include <functional>

class WebsocketClient {
    int socketFd;
    int responseMaxLength = 65536 * 3;
    std::thread messageThread;
    std::function<void(char*, int, int)> callback;

    void createReadLoop();

    static std::string createUrl(std::string protocol, std::string hostname, int *port, std::string *path);
public:
    WebsocketClient() = default;
    ~WebsocketClient() = default;

    void connect(const std::string &protocol, const std::string &hostname, int *port, std::string *path);
    void setOnMessageCallback(std::function<void(char*, int, int)> callback);
    void start();
    void stop();

    // void setOnOpenCallback(std::function<void()> callback);
    // void setOnCloseCallback(std::function<void()> callback);
};