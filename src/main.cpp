#include <iostream>
#include <thread>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <mutex>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

constexpr int MAX_CLIENTS = 6;
constexpr int PORT = 12345;
constexpr int BUFFER_SIZE = 1024;

std::mutex mtx;
std::atomic<bool> exitServer{false};
std::atomic<int> numClients{0};
std::vector<SOCKET> clients;

// Generate a unique 32-bit ID
#include <iostream>
#include <chrono>
const uint32_t serverStartTime = static_cast<uint32_t>(std::time(nullptr));

uint32_t generateID()
{
    // Mesure du temps de début
    auto start = std::chrono::high_resolution_clock::now();

    static std::atomic<uint32_t> counter{0};
    //    uint32_t timestamp = static_cast<uint32_t>(std::time(nullptr));

    uint32_t id = (serverStartTime << 16) | counter.fetch_add(1);

    // Mesure du temps de fin
    auto end = std::chrono::high_resolution_clock::now();

    // Calcul de la durée d'exécution en nanosecondes
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    std::cout << "Runtime duration of generateID() : " << duration.count() << " nanoseconds" << std::endl;

    return id;
}

// Thread function to handle client connections
void clientThread(SOCKET clientSocket)
{
    char buffer[BUFFER_SIZE];

    // Send unique ID every second
    while (!exitServer)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Generate unique ID
        uint32_t id = generateID();

        // Convert ID to ASCII and add new line character
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(8) << std::hex << id << "\n\r";
        std::string message = ss.str();

        // Send ID to client
        if (send(clientSocket, message.c_str(), message.size(), 0) == SOCKET_ERROR)
        {
            std::cerr << "Error sending data to client" << std::endl;
            break;
        }
    }

    // Close client socket
    closesocket(clientSocket);

    {
        std::lock_guard<std::mutex> lock(mtx);
        numClients--;

        // Remove client from the list
        auto it = std::find(clients.begin(), clients.end(), clientSocket);
        if (it != clients.end())
        {
            clients.erase(it);
        }
    }

    std::cout << "Client disconnected" << std::endl;
}

// Thread function to listen for client messages
void listenThread(SOCKET clientSocket)
{
    char buffer[BUFFER_SIZE];

    while (!exitServer)
    {
        // Receive data from client
        int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead == SOCKET_ERROR)
        {
            std::cerr << "Error receiving data from client" << std::endl;
            break;
        }
        else if (bytesRead == 0)
        {
            // Client disconnected
            std::cout << "Client disconnected" << std::endl;
            break;
        }

        // Check if received data is a new line
        if (std::string(buffer, bytesRead).find('\n') != std::string::npos || std::string(buffer, bytesRead).find('\r') != std::string::npos)
        {
            std::cout << "New line detected from client" << std::endl;
            std::string message;
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::stringstream ss;
                ss << "Number of clients: " << numClients;
                message = ss.str() + "\n";
            }
            // broadcast number of clients to all clients
            for (const auto &clientSocket : clients)
            {
                if (send(clientSocket, message.c_str(), message.size(), 0) == SOCKET_ERROR)
                {
                    std::cerr << "Error sending data to client" << std::endl;
                }
            }
        }
    }
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return 1;
    }

    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    int clientLength = sizeof(clientAddress);

    // Create server socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        std::cerr << "Error creating server socket" << std::endl;
        WSACleanup();
        return 1;
    }

    // Set server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind server socket to address
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        std::cerr << "Error binding server socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for client connections
    if (listen(serverSocket, MAX_CLIENTS) == SOCKET_ERROR)
    {
        std::cerr << "Error listening for connections" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server started. Listening on port " << PORT << std::endl;

    // Accept and handle client connections
    while (!exitServer)
    {
        // Accept client connection
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLength)) == INVALID_SOCKET)
        {
            std::cerr << "Error accepting client connection" << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            if (numClients >= MAX_CLIENTS)
            {
                std::cerr << "Maximum number of clients reached" << std::endl;
                closesocket(clientSocket);
                continue;
            }

            numClients++;
            clients.push_back(clientSocket);
            std::cout << "Client " << numClients << " connected" << std::endl; // Print the client number
        }

        // Start a new thread to handle client
        std::thread client_thread(clientThread, clientSocket);
        client_thread.detach();

        // Start a new thread to listen for client messages
        std::thread listen_thread(listenThread, clientSocket);
        listen_thread.detach();
    }

    // Cleanup and exit
    closesocket(serverSocket);
    WSACleanup();
    std::cout << "Server stopped" << std::endl;

    return 0;
}
