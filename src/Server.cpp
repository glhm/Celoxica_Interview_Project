#include "Server.hpp"
#include "Output.hpp"

bool Server::init()
{
    // Create socket, reliable mode, IPv4 adress
    m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_server_socket == -1)
    {
        Output::shared_print("Failed to create socket ");
        return false;
    }

    // get file descriptor file associated with server socket
    int flags = fcntl(m_server_socket, F_GETFL, 0);
    if (flags == -1)
    {
        Output::shared_print("Failed to get socket flags ");
        return false;
    }
    flags |= O_NONBLOCK; // Flag to set the socket to non-blocking mode.
    int ret = fcntl(m_server_socket, F_SETFL, flags);
    if (ret == -1)
    {
        Output::shared_print("Failed to set socket flags ");
        return false;
    }
    int reuse = 1;
    // Enable address reuse for the socket
    if (setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        Output::shared_print("Failed to set socket options");
        return false;
    }
    // Structure for specifying the server address using IPv4
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address)); // set the memory of the server_address structure to zero.
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;    // IP address for binding the socket to all available network interfaces.
    server_address.sin_port = htons(m_port_number); // Port number in network byt

    // Bind the socket to the server address
    if (bind(m_server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        Output::shared_print("Failed to bind socket: ");
        return false;
    }
    // Listen for incoming connections on the socket, connection_queue size restrained to m_pending_connections

    if (listen(m_server_socket, m_pending_connections) == -1)
    {
        Output::shared_print("Failed to listen on socket: ");
        return false;
    }
    return true;
}

void Server::start()
{
    std::cout << "\n\t[ Serveur Started ] " << std::endl;

    struct sockaddr_in client;
    unsigned int len = sizeof(sockaddr_in);
    int client_socket;
    int id_client = 0;

    // loop till stop called
    while (m_shutdown_requested == false)
    {

        // Try accepting a new client (Non blocking) from queue
        client_socket = accept(m_server_socket, (struct sockaddr *)&client, &len);

        // if no pending connections
        if (client_socket == -1)
        {
            // check for specific error codes for non-blocking sockets,
            // Used to avoid blocking state
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // No pending connection at the moment OR should be retry later
                continue;
            }
            else
            { //
                Output::shared_print("Accept error");
                break;
            }
        }
        // connection accepted
        id_client++; // each time a client is connected it has a new id

        //  start asynchronous task to handle client ( listening for deconnection or messages)
        std::thread t([=]
                      { handle_client(client_socket, id_client); });
        //  start asynchronous periodic task to send unique id to client every second
        std::thread t1([=]
                       { periodic_send_of_unique_id(client_socket); });

        // mutex lock to protect critical section clients
        std::lock_guard<std::mutex> guard(m_clients_mtx);
        m_clients.push_back({id_client, client_socket, move(t), move(t1)});
    }

    // stop has been called here, wait for the asynchronous taskes to finish before closing properly
    for (auto &client : m_clients)
    {
        if (client.th_listen.joinable())
        {
            client.th_listen.join();
        }
        if (client.th_write.joinable())
        {
            client.th_write.join();
        }
    }
    // Here Stop has been called
    std::cout << "Close Server\n ";
    close(m_server_socket);
}

Server::Server()
    : m_unique_id_generator()
{
}

void Server::stop()
{
    broadcast_message("Thank you\n");
    m_shutdown_requested = true;
}

bool Server::send_message_to_client(int client_socket, const std::string &message)
{
    // send data over a connected socket ret = number of bytes sent
    ssize_t ret = send(client_socket, message.c_str(), message.size(), 0);
    if (ret == -1)
    {
        Output::shared_print("Socket closed");
        return false;
    }
    else
    { // data success
        return true;
    }
}
void Server::broadcast_message(const std::string &message)
{
    // acquire mutex
    std::lock_guard<std::mutex> lock(m_clients_mtx);
    perform_broadcast(message);
    // release mutex
}

void Server::perform_broadcast(const std::string &message)
{
    for (auto &client : m_clients)
    {
        send_message_to_client(client.socket, message);
    }
}
void Server::broadcast_client_count()
{
    // acquire mutex
    std::lock_guard<std::mutex> lock(m_clients_mtx);
    std::string message = "Number of clients " + std::to_string(m_clients.size()) + "\n";
    perform_broadcast(message);
}

void Server::end_client_connection(int client_socket, int id)
{ // acquire mutex
    std::lock_guard<std::mutex> guard(m_clients_mtx);
    // acquire mutex
    // search for the client to stop
    for (int i = 0; i < m_clients.size(); i++)
    {
        if (m_clients[i].socket == client_socket)
        {
            std::cout << "Client socket " << id << " close" << std::endl;
            // close socket
            close(m_clients[i].socket);
            // detach threads, and assume . Assumes they will terminate on their own
            m_clients[i].th_listen.detach();
            m_clients[i].th_write.detach();
            // remove client from list
            m_clients.erase(m_clients.begin() + i);
            Output::shared_print(std::to_string(m_clients.size()) + " clients remaining");
            break;
        }
    }
}
void Server::periodic_send_of_unique_id(int client_socket)
{
    while (m_shutdown_requested == false)
    {
        std::this_thread::sleep_for(std::chrono::seconds(m_write_period_in_s));
        std::string message = std::to_string(m_unique_id_generator.generateID()) + "\t\n";
        if (send_message_to_client(client_socket, message) == false)
        {
            // if could not write, client disconnected, exit the loop
            break;
        }
    }
}

void Server::handle_client(int client_socket, int id)
{
    char str[m_max_buffer_length]; // buffer to store data received from client
    // Display welcome message on output and send to client
    std::string client_entered_message = "Client " + std::to_string(id) + " has joined\n";
    Output::shared_print(client_entered_message);
    broadcast_message(client_entered_message);

    std::string press_enter_message = "Press Enter to get the number of clients\n";
    send_message_to_client(client_socket, press_enter_message);

    // loop till Stop is called
    while (m_shutdown_requested == false)
    {
        // Receive bytes data from the client (non-blocking)
        int bytes_received = recv(client_socket, str, sizeof(str), MSG_DONTWAIT); // non bloquant

        // if no bytes, Verify if connection is interrupted or error occurred
        if (bytes_received == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // No data available at the moment, continue checking
                continue;
            }
            else
            { // if other error than those accepted
                Output::shared_print("Failed to receive data: ");
                break;
            }
        }
        else if (bytes_received == 0)
        { // client disconnected
            Output::shared_print("Connection interrupted by client " + std::to_string(id));
            break;
        }
        // if data successfully received, verify if data contains newline character
        if (std::string(str, bytes_received).find('\n') != std::string::npos || std::string(str, bytes_received).find('\r') != std::string::npos)
        {
            Output::shared_print("Number of clients asked by client " + std::to_string(id) + ", broadcast to all");
            Output::shared_print("Broadcast Client Count");
            broadcast_client_count();
        }
    }
    // Stopped asked here or client disconnected
    end_client_connection(client_socket, id);
}