#pragma once

#include <atomic>
#include <mutex>
#include <vector>
#include <cstdint>

constexpr int MAX_CLIENTS = 6;
constexpr int BUFFER_SIZE = 1024;
constexpr int PORT = 12345;

class Server {
public:
    Server();
    ~Server();

    void start();
    void stop();

private:
    const uint32_t serverStartTime = static_cast<uint32_t>(std::time(nullptr));

    std::atomic<bool> exitServer;
    std::atomic<int> numClients;
    std::vector<int> clients;

    std::mutex mtx;

    uint32_t generateID();
    void signalHandler(int signal);
    void clientThread(int clientSocket);
    void listenThread(int clientSocket);
};
