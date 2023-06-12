
#include <iostream>
#include <csignal>
#include <memory>
#include "Server.hpp"
// Déclaration de la fonction de rappel pour SIGINT
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
    signal(SIGINT, signalHandler);

    server = std::make_unique<Server>();
    if (server->start())
    {
        std::cout << "Error Starting server, abort" << std::endl;
    }

    return 0;
}