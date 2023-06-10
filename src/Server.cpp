#include "Server.hpp"
#include <iostream>
#include <thread>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <atomic>


Server::Server() : numClients(0), exitServer(false) {}

Server::~Server() {}

void Server::start() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientLength = sizeof(clientAddress);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating server socket" << std::endl;
        return;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding server socket" << std::endl;
        close(serverSocket);
        return;
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        std::cerr << "Error listening for connections" << std::endl;
        close(serverSocket);
        return;
    }

    std::cout << "Server started. Listening on port " << PORT << std::endl;

    while (!exitServer.load()) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLength);
        if (clientSocket == -1) {
            std::cerr << "Error accepting client connection" << std::endl;
            close(serverSocket);
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            if (numClients >= MAX_CLIENTS) {
                std::cerr << "Maximum number of clients reached" << std::endl;
                close(clientSocket);
                continue;
            }

            numClients++;
            clients.push_back(clientSocket);
            std::cout << "Client " << numClients << " connected" << std::endl;
        }

        std::thread clientThread(&Server::clientThread, this, clientSocket);
        clientThread.detach();

        std::thread listenThread(&Server::listenThread, this, clientSocket);
        listenThread.detach();
    }

    close(serverSocket);
    std::cout << "Server stopped" << std::endl;
}

void Server::stop() {
    exitServer.store(true);
}

uint32_t Server::generateID() {
    auto start = std::chrono::high_resolution_clock::now();

    static std::atomic<uint32_t> counter{0};
    uint32_t id = (serverStartTime << 16) | counter.fetch_add(1);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    std::cout << "Runtime duration of generateID(): " << duration.count() << " nanoseconds" << std::endl;

    return id;
}

void Server::clientThread(int clientSocket) {
    char buffer[BUFFER_SIZE];

    while (!exitServer.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        uint32_t id = generateID();

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(8) << std::hex << id << "\n\r";
        std::string message = ss.str();

        if (send(clientSocket, message.c_str(), message.size(), 0) == -1) {
            std::cerr << "Error sending data to client" << std::endl;
            break;
        }
    }

    close(clientSocket);

    {
        std::lock_guard<std::mutex> lock(mtx);
        numClients--;

        auto it = std::find_if(clients.begin(), clients.end(), [clientSocket](int socket) {
            return socket == clientSocket;
        });

        if (it != clients.end()) {
            clients.erase(it);
        }
    }

    std::cout << "On passe ici Client disconnected" << std::endl;
}

void Server::listenThread(int clientSocket) {
    char buffer[BUFFER_SIZE];

    while (!exitServer.load()) {
        int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead == -1) {
            std::cerr << "Error receiving data from client" << std::endl;
            break;
        }
        else if (bytesRead == 0) {
            std::cout << "On passe ici 1 Client disconnected" << std::endl;
            break;
        }

        if (std::string(buffer, bytesRead).find('\n') != std::string::npos || std::string(buffer, bytesRead).find('\r') != std::string::npos) {
            std::cout << "New line detected from client" << std::endl;
            std::string message;
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::stringstream ss;
                ss << "Number of clients: " << numClients;
                message = ss.str() + "\n";
            }
            // Broadcast number of clients to all clients
            for (const auto& client : clients) {
                if (client != clientSocket) {
                    if (send(client, message.c_str(), message.size(), 0) == -1) {
                        std::cerr << "Error sending data to client" << std::endl;
                    }
                }
            }
        }
    }

    close(clientSocket);
    std::cout << "On passe ici 2Client disconnected" << std::endl;
}
