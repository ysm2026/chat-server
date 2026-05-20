#include "Client.h"
#include <iostream>
#include <cstring>
#include <sstream>

#ifdef _WIN32
    #define CLOSE_SOCKET(fd) closesocket(fd)
#else
    #define CLOSE_SOCKET(fd) close(fd)
#endif

static const int BUFFER_SIZE = 4096;

Client::Client()
    : sock_(INVALID_SOCKET)
    , connected_(false)
{}

Client::~Client() {
    connected_ = false;
    if (sock_ != INVALID_SOCKET) {
        CLOSE_SOCKET(sock_);
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

bool Client::connect(const std::string& host, int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[ERROR] WSAStartup failed" << std::endl;
        return false;
    }
#endif

    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "[ERROR] Failed to create socket" << std::endl;
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "[ERROR] Invalid address: " << host << std::endl;
        return false;
    }

    if (::connect(sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[ERROR] Connection to " << host << ":" << port << " failed" << std::endl;
        return false;
    }

    connected_ = true;
    std::cout << "[INFO] Connected to " << host << ":" << port << std::endl;
    return true;
}

void Client::run() {
    std::thread listener(&Client::listenThread, this);

    std::string line;
    while (connected_ && std::getline(std::cin, line)) {
        if (sock_ != INVALID_SOCKET) {
            std::string to_send = line + "\n";
            send(sock_, to_send.c_str(), to_send.size(), 0);
        }
    }

    connected_ = false;
    if (listener.joinable()) {
        listener.join();
    }
}

void Client::listenThread() {
    char buf[BUFFER_SIZE];
    while (connected_) {
        int n = recv(sock_, buf, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            if (connected_) {
                std::cout << "\n[INFO] Disconnected from server." << std::endl;
            }
            break;
        }
        buf[n] = '\0';
        std::cout << buf;
        std::cout.flush();
    }
    connected_ = false;
}
