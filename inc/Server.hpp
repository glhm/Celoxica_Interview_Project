#ifndef SERVER_HPP
#define SERVER_HPP
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <map>

#include <fcntl.h>
#include <atomic>
#include "UniqueIdGenerator.hpp"
// Structure to modelize a Server-side client
struct Client
{
    int socket;            // socket associated for the server to connect to
    std::thread th_listen; // thread in charge of polling for client messages
    std::thread th_write;  // thread in charge of writing periodically to the client
};

/*
 * Class implementing a TCP Server using non-blocking sockets
 * Allows up to 6 clients to connect to receive a unique ID
 * Each client can ask for the number of clients by sending a newline character
 */
class Server
{
public:
    Server();

    /* Initialize the server by creating a socket, and binding it to the server address
     */
    bool init();
    /*
     * Stop the server by stopping all the client related threads,
     * closing the client sockets and finally closing the server
     * @return if value -1 init failed
     */
    void stop();

    /**
     * Starts the server by creating client sockets and allowing new clients to connect,
     *  then starts polling new data from client and sending data periodically
     */
    void start();

    /**
     * Polls the data from a client
     * @param id number of the client for displaying
     */
    void handle_client(int id);

    /**
     * Send to client socket unique ids periodically
     * @param client_socket socket on which data is sent
     */
    void periodic_send_of_unique_id(int client_socket);

private:
    /**
     * Send data to a client over a socket
     * @param client_socket socket on which data is sent
     * @param message message to send
     * @return true if success, false otherwise
     */
    bool send_message_to_client(int client_socket, const std::string &message);

    /**
     * Close client socket and detach related threads
     * @param id id of the client
     */
    void end_client_connection(int id);

    /**
     * Calls perform_broadcast after locking clients_mtx
     * @param message message to broadcast
     */
    void broadcast_message(const std::string &message);

    /**
     * Sends data to a clients
     * @param message message to broadcast
     */
    void perform_broadcast(const std::string &message);

    /**
     * Sends number of clients to all clients
     * @param message message to broadcast
     */
    void broadcast_client_count();

    UniqueIdGenerator m_unique_id_generator; // generator of ids to send periodically to clients

    std::map<int, Client> m_clients;               // current clients connected map id of clients, structure of clients
    std::atomic<bool> m_shutdown_requested{false}; // flag which is raised when Stop is asked
    std::mutex m_clients_mtx;                      // mutex to avoid data races when accessing client shared data
    int m_server_socket;                           // file descriptor of a socket, describes a communication endpoints
    /**
     * Configuration
     */
    const int m_max_number_of_clients = 6; // max number of clients allowed by the Server
    const int m_pending_connections = 8;   // max size of queue of waiting clients
    const int m_write_period_in_s = 1;     // period of write
    const int m_max_buffer_length = 200;   // max length of buffer for receiving data
    const int m_port_number = 12345;       // port number on which the server is listening
};

#endif