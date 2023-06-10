#include <memory>
#include <csignal>
#include "Server.hpp"
#include <iostream>
#include <csignal>


Server serverInstance;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Received SIGINT signal. Stopping server..." << std::endl;
        serverInstance.stop();
    }
}

int main() {
    // ...

    // Configure signal handler for SIGINT
    std::signal(SIGINT, signalHandler);

    // Start the server
    serverInstance.start();

    return 0;
}

