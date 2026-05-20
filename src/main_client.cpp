#include "Client.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    int port = 8888;

    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = std::atoi(argv[2]);

    Client client;
    if (!client.connect(host, port)) {
        return 1;
    }

    client.run();
    return 0;
}
