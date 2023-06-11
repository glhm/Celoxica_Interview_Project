#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#define MAX_LEN 200
#define NUM_COLORS 6

using namespace std;

struct terminal
{
    int id;
    string name;
    int socket;
    thread th;
};

vector<terminal> clients;
string def_col = "\033[0m";
string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};
int seed = 0;
mutex cout_mtx, clients_mtx;
int server_socket;

string color(int code);
void set_name(int id, char name[]);
void stop();
void shared_print(string str, bool endLine);
void broadcast_message(string message);
void broadcast_client_count();
void end_connection(int id);
void handle_client(int client_socket, int id);

// Déclaration de la fonction de rappel pour SIGINT
void signalHandler(int signum)
{
    if (signum == SIGINT)
    {
        stop(); // Appel de la fonction stop() pour arrêter le serveur proprement
        // exit(signum); // Terminer le programme avec le code de sortie du signal
    }
}

int main()
{
    signal(SIGINT, signalHandler);

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket: ");
        exit(-1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(10000);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&server.sin_zero, 0);

    if ((bind(server_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) == -1)
    {
        perror("bind error: ");
        exit(-1);
    }

    if ((listen(server_socket, 8)) == -1)
    {
        perror("listen error: ");
        exit(-1);
    }

    struct sockaddr_in client;
    int client_socket;
    unsigned int len = sizeof(sockaddr_in);

    cout << colors[NUM_COLORS - 1] << "\n\t  ====== Welcome ======   " << endl
         << def_col;

    while (1)
    {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client, &len)) == -1)
        {
            perror("accept error: ");
            exit(-1);
        }
        seed++;
        thread t(handle_client, client_socket, seed);
        lock_guard<mutex> guard(clients_mtx);
        clients.push_back({seed, string("Anonymous"), client_socket, (move(t))});
    }

    // for (int i = 0; i < clients.size(); i++)
    // {
    //     if (clients[i].th.joinable())
    //         clients[i].th.join();
    // }

    // close(server_socket);
    return 0;
}

string color(int code)
{
    return colors[code % NUM_COLORS];
}

void stop()
{
    std::cout << "Stop Asked " << std::endl;
    lock_guard<mutex> guard(clients_mtx);
    auto clients_it = clients.begin();
    while (clients_it != clients.end())
    {
        // supprimer les nombres impairs
        std::cout << "Close socket client " << clients_it->id << std::endl;

        // close(clients_it->socket); // Ferme la socket du client
        shutdown(clients_it->socket, SHUT_RD); // Ferme la socket du client en lecture

        std::cout << "Will stop " << clients_it->id << std::endl;

        if (clients_it->th.joinable())
        {
            std::cout << "Join thread " << clients_it->id << std::endl;
            clients_it->th.join();
            // supprimer le thread de la liste
            std::cout << "Erase it from the list \n";

            clients_it = clients.erase(clients_it);
        }
        else
        {
            std::cout << "Thread Client already terminated " << clients_it->id << ", next one \n ";
            ++clients_it;
        }
    }
    close(server_socket);
}

// Set name of client
void set_name(int id, char name[])
{
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id == id)
        {
            clients[i].name = string(name);
        }
    }
}

// For synchronisation of cout statements
void shared_print(string str, bool endLine = true)
{
    lock_guard<mutex> guard(cout_mtx);
    cout << str;
    if (endLine)
        cout << endl;
}

void send_message_to_client(int client_socket, string message)
{
    ssize_t ret = send(client_socket, message.c_str(), message.size(), 0);
    if (ret == -1)
    {
        cerr << "Error sending data to client" << endl;
        return;
    }
}
// Broadcast message to all clients except the sender
void broadcast_message(string message)
{
    for (int i = 0; i < clients.size(); i++)
    {
        send_message_to_client(clients[i].socket, message);
    }
}

void broadcast_client_count()
{
    std::string message;
    std::stringstream ss;

    {
        std::lock_guard<std::mutex> lock(clients_mtx);
        int num_clients = clients.size(); // attention a la synchronisation
        ss << "Number of clients: " << std::to_string(num_clients);
        message = ss.str() + "\n";
    }
    broadcast_message(message);
}

void end_connection(int id)
{
    std::cout << "Client " << id << " quitting" << std::endl;
    lock_guard<mutex> guard(clients_mtx);

    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id == id)
        {
            close(clients[i].socket); // Ferme la socket du client

            {
                clients[i].th.detach();
                clients.erase(clients.begin() + i);
            }

            break;
        }
    }
}

void handle_client(int client_socket, int id)
{
    char name[MAX_LEN], str[MAX_LEN];
    // recoit le nom du client
    recv(client_socket, name, sizeof(name), 0);
    set_name(id, name);

    // Display welcome message
    string client_entered_message = "Client " + std::to_string(id) + string(" has joined") + "\n";

    broadcast_message(client_entered_message);

    string press_enter_message = "Press Enter to get the number of clients\n";
    send_message_to_client(client_socket, press_enter_message);

    shared_print(color(id) + client_entered_message + def_col);

    // gere les messages du client
    while (1)
    {
        int bytes_received = recv(client_socket, str, sizeof(str), 0);
        // verify if connection interrupted or error
        if (bytes_received <= 0)
        {
            shared_print(color(id) + "Connection interrupted with cient " + std::to_string(id));
            end_connection(id);
            shared_print(color(id) + std::to_string(clients.size()) + " clients remaining");

            return;
        }
        // if (strcmp(str, "#exit") == 0)
        // {
        //     // Display leaving message
        //     string message = string(name) + string(" has left");
        //     broadcast_message("#NULL", id);
        //     broadcast_message(message, id);
        //     shared_print(color(id) + message + def_col);
        //     end_connection(id);
        //     return;
        // }
        // verify if character is a new line
        if (std::string(str, bytes_received).find('\n') != std::string::npos || std::string(str, bytes_received).find('\r') != std::string::npos)
        {
            shared_print(color(id) + "New line detected from client " + std::to_string(id) + def_col);
            shared_print("Broadcast Client Count");
            broadcast_client_count();
        }
    }
    std::cout << "Fin thread Handle Client<<" << std::endl;
}