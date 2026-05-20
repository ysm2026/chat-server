#include "Server.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>

#ifdef _WIN32
    #define CLOSE_SOCKET(fd) closesocket(fd)
#else
    #define CLOSE_SOCKET(fd) close(fd)
#endif

static const int BUFFER_SIZE = 4096;

Server::Server(int port)
    : port_(port)
    , server_fd_(INVALID_SOCKET)
    , running_(false)
{}

Server::~Server() {
    running_ = false;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto fd : clients_) {
            CLOSE_SOCKET(fd);
        }
        clients_.clear();
    }
    if (server_fd_ != INVALID_SOCKET) {
        CLOSE_SOCKET(server_fd_);
    }

#ifdef _WIN32
    WSACleanup();
#endif
}

void Server::start() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[ERROR] WSAStartup failed" << std::endl;
        return;
    }
#endif

    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == INVALID_SOCKET) {
        std::cerr << "[ERROR] Failed to create socket" << std::endl;
        return;
    }

    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&opt), sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "[WARN] Failed to set SO_REUSEADDR" << std::endl;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(server_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[ERROR] Bind failed on port " << port_ << std::endl;
        return;
    }

    if (listen(server_fd_, 5) == SOCKET_ERROR) {
        std::cerr << "[ERROR] Listen failed" << std::endl;
        return;
    }

    running_ = true;
    std::cout << "[INFO] Chat server started on port " << port_ << std::endl;
    std::cout << "[INFO] Waiting for connections..." << std::endl;

    while (running_) {
        sockaddr_in client_addr{};
#ifdef _WIN32
        int addr_len = sizeof(client_addr);
#else
        socklen_t addr_len = sizeof(client_addr);
#endif
        socket_t client_fd = accept(server_fd_,
                                    reinterpret_cast<sockaddr*>(&client_addr),
                                    &addr_len);
        if (client_fd == INVALID_SOCKET) {
            if (running_) {
                std::cerr << "[ERROR] Accept failed" << std::endl;
            }
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        std::cout << "[INFO] New connection from " << client_ip << std::endl;

        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            clients_.insert(client_fd);
        }

        std::thread(&Server::handleClient, this, client_fd).detach();
    }
}

void Server::handleClient(socket_t client_fd) {
    char buf[BUFFER_SIZE];
    std::string username;
    bool named = false;

    // Ask for username
    std::string prompt = "Welcome to Chat Server!\nEnter your username: ";
    send(client_fd, prompt.c_str(), prompt.size(), 0);

    while (running_) {
        int n = recv(client_fd, buf, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            break;
        }
        buf[n] = '\0';

        std::string msg(buf);

        // Trim whitespace
        msg.erase(0, msg.find_first_not_of(" \t\r\n"));
        msg.erase(msg.find_last_not_of(" \t\r\n") + 1);

        if (msg.empty()) continue;

        if (!named) {
            if (msg.empty() || msg.find(' ') != std::string::npos) {
                std::string err = "Invalid username. Try again: ";
                send(client_fd, err.c_str(), err.size(), 0);
                continue;
            }
            username = msg;
            named = true;
            std::string notice = "[SERVER] " + username + " joined the chat!";
            std::cout << notice << std::endl;
            broadcast(notice);
            continue;
        }

        // Normal message
        std::string formatted = "[" + username + "] " + msg;
        std::cout << formatted << std::endl;
        broadcast(formatted, client_fd);
    }

    // Client disconnected
    cleanupSocket(client_fd);

    if (named) {
        std::string notice = "[SERVER] " + username + " left the chat.";
        std::cout << notice << std::endl;
        broadcast(notice);
    }
}

void Server::broadcast(const std::string& msg, socket_t exclude) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto fd : clients_) {
        if (fd == exclude) continue;
        std::string to_send = msg + "\n";
        send(fd, to_send.c_str(), to_send.size(), 0);
    }
}

void Server::cleanupSocket(socket_t fd) {
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        clients_.erase(fd);
    }
    CLOSE_SOCKET(fd);
}
