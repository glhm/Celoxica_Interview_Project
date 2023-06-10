#ifndef SERVER
#define SERVER
#pragma once

#include <atomic>
#include <mutex>
#include <vector>
#include <cstdint>
#include "UniqueIdGenerator.hpp"

constexpr int MAX_CLIENTS = 6;
constexpr int BUFFER_SIZE = 1024;
constexpr int PORT = 12345;

class Server
{
public:
    Server();
    ~Server();

    void start();
    void stop();

private:
    UniqueIdGenerator m_uniqueIdGenerator;
    std::atomic<bool> exitServer;
    std::atomic<int> numClients;
    std::vector<int> clients;

    std::mutex mtx;

    void signalHandler(int signal);
    void clientThread(int clientSocket);
    void listenThread(int clientSocket);
};

#endif