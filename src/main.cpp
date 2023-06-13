
#include <iostream>
#include <csignal>
#include <memory>
#include "Server.hpp"
// global pointer to call stop from signalHandler
std::unique_ptr<Server> server;

void signalHandler(int signum)
{
    if (signum == SIGINT)
    {
        if (server)
            server->stop(); // Appel de la fonction stop() pour arrêter le serveur proprement
                            // exit(signum); // Terminer le programme avec le code de sortie du signal
    }
}

int main()
{
    signal(SIGINT, signalHandler); // bind signal emitted from Ctrl + C to signal void signalHandler(int signum)

    server = std::make_unique<Server>();

    if (server->init() == -1)
    {
        return 0;
    }

    if (server->start() == -1)
    {
        std::cout << "Error Starting server, finish" << std::endl;
        return 0;
    }

    return 0;
}