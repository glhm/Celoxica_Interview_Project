
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
        {
            server->stop(); // Appel de la fonction stop() pour arrêter le serveur proprement
        }
    }
}

int main()
{
    signal(SIGINT, signalHandler); // bind signal emitted from Ctrl + C to signal void signalHandler(int signum)

    server = std::make_unique<Server>();

    if (server->init() == false)
    {
        return 0;
    }
    else
    {
        server->start();
    }

    return 0;
}