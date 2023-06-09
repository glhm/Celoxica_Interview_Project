#include "Server.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

Server::Server() : m_socket(INVALID_SOCKET), m_running(false), m_clientCount(0)
{
    std::cout << "Server created" << std::endl;
}

Server::~Server()
{
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
    }
}

void Server::start()
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock\n";
        return;
    }

    // Create socket
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return;
    }

    // Set socket options
    int enable = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&enable), sizeof(int)) == SOCKET_ERROR)
    {
        std::cerr << "Failed to set socket options\n";
        closesocket(m_socket);
        WSACleanup();
        return;
    }

    // Bind socket to port
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (bind(m_socket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
    {
        std::cerr << "Failed to bind socket\n";
        closesocket(m_socket);
        WSACleanup();
        return;
    }

    // Start listening for incoming connections
    if (listen(m_socket, MAX_CLIENTS) == SOCKET_ERROR)
    {
        std::cerr << "Failed to start listening\n";
        closesocket(m_socket);
        WSACleanup();
        return;
    }

    m_running = true;
    std::cout << "Server started, listening on port " << PORT << std::endl;

    while (m_running)
    {
        // Accept new client connection
        sockaddr_in clientAddress{};
        int clientAddressLength = sizeof(clientAddress);
        SOCKET clientSocket = accept(m_socket, reinterpret_cast<struct sockaddr *>(&clientAddress), &clientAddressLength);

        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Failed to accept client connection\n";
            continue;
        }

        std::cout << "New client connected" << std::endl;

        // Check if the maximum number of clients has been reached
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_clientCount >= MAX_CLIENTS)
        {
            std::cerr << "Maximum number of clients reached\n";
            lock.unlock();
            closesocket(clientSocket);
            continue;
        }

        // Create a new thread for the client
        std::thread clientThread(&Server::handleClient, this, clientSocket);
        clientThread.detach();

        // Increment the client count
        ++m_clientCount;
    }

    // Close the socket when done
    closesocket(m_socket);

    // Cleanup Winsock
    WSACleanup();
    std::cout << "Server stopped" << std::endl;
}

void Server::handleClient(SOCKET clientSocket)
{
    std::cout << "Handling client" << std::endl;

    std::string uniqueID = generateUniqueID();
    std::cout << "Sending unique ID: " << uniqueID << std::endl;

    while (true)
    {
        // Send the unique ID to the client
        if (send(clientSocket, uniqueID.c_str(), static_cast<int>(uniqueID.length()), 0) == SOCKET_ERROR)
        {
            std::cerr << "Failed to send unique ID to client\n";
            break;
        }

        // Terminate the ID with a new line character
        char newline = '\n';
        if (send(clientSocket, &newline, sizeof(newline), 0) == SOCKET_ERROR)
        {
            std::cerr << "Failed to send newline character to client\n";
            break;
        }

        // Wait for 1 second before sending the ID again
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Close the client socket
    closesocket(clientSocket);

    // Decrement the client count
    std::lock_guard<std::mutex> lock(m_mutex);
    --m_clientCount;

    std::cout << "Client handling complete" << std::endl;
}

std::string Server::generateUniqueID()
{
    // Generate a unique ID here
    // For testing purposes, we'll use a simple counter
    static int counter = 0;
    return std::to_string(counter++);
}
