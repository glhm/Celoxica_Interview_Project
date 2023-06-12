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
#include <fcntl.h>
#include <vector>
#include <atomic>

// Réecrire la classe Server de main ici
// Vous pouvez ajouter des fonctions membres si vous le souhaitez

#define MAX_LEN 200
#define NUM_COLORS 6
#define PORT_NUMBER 12345

struct Client
{
    int id;
    int socket;
    std::thread th;
    std::thread th_write;
};

class Server
{
public:
    Server();
    int init();
    void stop();
    int start();

    void handle_client(int, int);
    void periodic_send_of_unique_id(int);

private:
    std::vector<Client> clients;
    std::atomic<bool> shutdown_requested{false};
    std::mutex cout_mtx, clients_mtx;

    std::string def_col = "\033[0m";
    std::vector<std::string> colors;
    int seed = 0;
    int server_socket;

    std::string color(int);
    void shared_print(const std::string &);
    void send_message_to_client(int, const std::string &);

    void broadcast_message(const std::string &);
    void broadcast_client_count();
    void end_connection(int);
};

#endif