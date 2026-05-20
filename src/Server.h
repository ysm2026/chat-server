#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <string>
#include <thread>
#include <mutex>
#include <unordered_set>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    constexpr socket_t INVALID_SOCKET = -1;
    constexpr int SOCKET_ERROR = -1;
#endif

class Server {
public:
    explicit Server(int port = 8888);
    ~Server();

    void start();
    void broadcast(const std::string& msg, socket_t exclude = INVALID_SOCKET);

private:
    void handleClient(socket_t client_fd);
    void cleanupSocket(socket_t fd);
    std::string welcomeMessage(const std::string& username) const;

    int port_;
    socket_t server_fd_;
    bool running_;
    std::mutex clients_mutex_;
    std::unordered_set<socket_t> clients_;
};

#endif // CHAT_SERVER_H
