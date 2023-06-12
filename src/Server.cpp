#include "Server.hpp"
int Server::init()
{
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return 1;
    }

    int flags = fcntl(server_socket, F_GETFL, 0);
    if (flags == -1)
    {
        std::cerr << "Failed to get socket flags: " << strerror(errno) << std::endl;
        return 1;
    }
    flags |= O_NONBLOCK;
    int ret = fcntl(server_socket, F_SETFL, flags);
    if (ret == -1)
    {
        std::cerr << "Failed to set socket flags: " << strerror(errno) << std::endl;
        return 1;
    }
    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        std::cerr << "Failed to set socket options: " << strerror(errno) << std::endl;
        return 1;
    }
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT_NUMBER);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        return 1;
    }

    if (listen(server_socket, 8) == -1)
    {
        std::cerr << "Failed to listen on socket: " << strerror(errno) << std::endl;
        return 1;
    }
    return 0;
}

int Server::start()
{

    struct sockaddr_in client;
    unsigned int len = sizeof(sockaddr_in);

    std::cout << "\n\t  ====== Serveur Started ======   " << std::endl;
    int client_socket;
    while (shutdown_requested == false)
    {
        // Tentative d'acceptation d'une nouvelle connexion
        client_socket = accept(server_socket, (struct sockaddr *)&client, &len);
        if (client_socket == -1)
        {
            // Gérer les erreurs d'acceptation
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Aucune connexion en attente, continuer à vérifier
                continue;
            }
            else
            {
                perror("accept error: ");
                exit(-1);
            }
        }
        seed++;
        std::thread t([=]
                      { handle_client(client_socket, seed); });
        std::thread t1([=]
                       { periodic_send_of_unique_id(client_socket); });

        std::lock_guard<std::mutex> guard(clients_mtx);
        clients.push_back({seed, client_socket, move(t), move(t1)});
    }
    std::cout << "End Main " << std::endl;
    for (auto &client : clients)
    {
        if (client.th.joinable())
        {
            std::cout << "Join thread " << client.id << std::endl;
            client.th.join();
        }
        if (client.th_write.joinable())
        {
            std::cout << "Join Writing thread " << client.id << std::endl;
            client.th_write.join();
        }

    } // Ferme la socket du server
    std::cout << "Close Server\n ";

    close(server_socket);
    return 0;
}

std::string Server::color(int code)
{
    return colors[code % NUM_COLORS];
}
Server::Server()
    : colors{"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"}
{
    init();
}

void Server::stop()
{
    broadcast_message("Thank you\n");
    shutdown_requested = true;
}

// For synchronisation of cout statements
void Server::shared_print(const std::string &str)
{
    std::lock_guard<std::mutex> guard(cout_mtx);
    std::cout << str << std::endl;
}

void Server::send_message_to_client(int client_socket, const std::string &message)
{
    ssize_t ret = send(client_socket, message.c_str(), message.size(), 0);
    if (ret == -1)
    {
        std::cerr << "Error sending data to client" << std::endl;
        return;
    }
}
// Broadcast message to all clients except the sender
void Server::broadcast_message(const std::string &message)
{
    for (int i = 0; i < clients.size(); i++)
    {
        send_message_to_client(clients[i].socket, message);
    }
}

void Server::broadcast_client_count()
{
    std::string message;
    {
        std::lock_guard<std::mutex> lock(clients_mtx);
        int num_clients = clients.size(); // attention a la synchronisation
        message + "Number of clients: " + std::to_string(num_clients);
    }
    broadcast_message(message);
}

void Server::end_connection(int id)
{
    std::cout << "Client " << id << " quitting" << std::endl;
    std::lock_guard<std::mutex> guard(clients_mtx);
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id == id)
        {
            std::cout << "Client socket " << id << " close" << std::endl;
            close(clients[i].socket); // Ferme la socket du client

            clients[i].th.detach();
            clients[i].th_write.detach();

            clients.erase(clients.begin() + i);
            break;
        }
    }
}
void Server::periodic_send_of_unique_id(int client_socket)
{
    while (shutdown_requested == false)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::string message = "Your unique id is 1";
        send_message_to_client(client_socket, message);
    }
    std::cout << "End Writing thread\n";
}

void Server::handle_client(int client_socket, int id)
{
    char str[MAX_LEN];

    // Display welcome message
    std::string client_entered_message = "Client " + std::to_string(id) + "has joined\n";
    broadcast_message(client_entered_message);

    std::string press_enter_message = "Press Enter to get the number of clients\n";
    send_message_to_client(client_socket, press_enter_message);
    shared_print(color(id) + client_entered_message + def_col);

    // gere les messages du client
    while (shutdown_requested == false)
    {
        int bytes_received = recv(client_socket, str, sizeof(str), MSG_DONTWAIT); // non bloquant
                                                                                  // verify if connection interrupted or error
        if (bytes_received == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Aucune donnée disponible pour le moment, continuer à vérifier
                // ou effectuer d'autres opérations
                continue;
            }
            else
            {
                std::cerr << "Failed to receive data: " << strerror(errno) << std::endl; // i
                shared_print(color(id) + "Error receiving data with client " + std::to_string(id));
                // return;
                break;
            }
        }
        else if (bytes_received == 0)
        {
            shared_print(color(id) + "Connection interrupted by cient " + std::to_string(id));
            break;
        }
        // verify if character is a new line
        if (std::string(str, bytes_received).find('\n') != std::string::npos || std::string(str, bytes_received).find('\r') != std::string::npos)
        {
            shared_print(color(id) + "New line detected from client " + std::to_string(id) + def_col);
            shared_print("Broadcast Client Count");
            broadcast_client_count();
        }
    }
    std::cout << "Fin thread Handle Client " << id << std::endl;
    end_connection(id);
    shared_print(color(id) + std::to_string(clients.size()) + " clients remaining");
}