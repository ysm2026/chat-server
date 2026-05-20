#include "Server.h"
#include <iostream>
#include <csignal>

static Server* g_server = nullptr;

void signalHandler(int) {
    std::cout << "\n[INFO] Shutting down server..." << std::endl;
    if (g_server) {
        delete g_server;
        g_server = nullptr;
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    int port = 8888;
    if (argc >= 2) {
        port = std::atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Invalid port. Using default 8888." << std::endl;
            port = 8888;
        }
    }

    signal(SIGINT, signalHandler);

    g_server = new Server(port);
    g_server->start();

    delete g_server;
    return 0;
}
