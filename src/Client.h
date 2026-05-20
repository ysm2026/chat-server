#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <string>
#include <thread>

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

class Client {
public:
    Client();
    ~Client();

    bool connect(const std::string& host, int port);
    void run();

private:
    void listenThread();
    void cleanupSocket(socket_t fd);

    socket_t sock_;
    bool connected_;
};

#endif
