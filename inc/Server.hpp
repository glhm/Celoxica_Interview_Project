#ifndef SERVER_HPP
#define SERVER_HPP

#include <mutex>
#include <string>
#include <winsock2.h>

constexpr int MAX_CLIENTS = 6;
constexpr int PORT = 12345;
constexpr int ID_INTERVAL_SECONDS = 1;

class Server
{
public:
    Server();
    ~Server();

    void start();

private:
    SOCKET m_socket;
    bool m_running;
    int m_clientCount;
    std::mutex m_mutex;

    void handleClient(SOCKET clientSocket);
    std::string generateUniqueID();
};

#endif // SERVER_HPP
