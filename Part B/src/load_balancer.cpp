/**
 * @file load_balancer.cpp
 * @brief Implementation of a load balancer for BlinkDB servers
 * @author Madhumita
 * @date 2025-03-31
 * 
 * This file implements a load balancer that distributes client connections
 * between multiple BlinkDB server instances using a round-robin algorithm.
 */
 
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

/**
 * @class LoadBalancer
 * @brief Implements a round-robin load balancer for multiple backend servers
 * 
 * This class distributes incoming client connections between multiple backend
 * servers using a round-robin algorithm. It forwards data between clients and
 * backend servers transparently.
 */
class LoadBalancer {
private:
    /**
     * @brief Server socket file descriptor
     */
    int server_fd;
    
    /**
     * @brief Server address structure
     */
    struct sockaddr_in address;
    
    /**
     * @brief Port number the load balancer listens on
     */
    int PORT;
    
    /**
     * @brief Maximum number of simultaneous client connections
     */
    static const int MAX_CLIENTS = 2000;
    
    /**
     * @brief IP address of the first backend server
     */
    std::string server1_ip;
    
    /**
     * @brief Port number of the first backend server
     */
    int server1_port;
    
    /**
     * @brief IP address of the second backend server
     */
    std::string server2_ip;
    
    /**
     * @brief Port number of the second backend server
     */
    int server2_port;
    
    /**
     * @brief Counter for round-robin server selection
     */
    int current_server;

public:
    /**
     * @brief Constructor
     * @param port Port number for the load balancer
     * @param s1_ip IP address of the first backend server
     * @param s1_port Port number of the first backend server
     * @param s2_ip IP address of the second backend server
     * @param s2_port Port number of the second backend server
     * 
     * Initializes the load balancer with the specified port and backend server details.
     */
    LoadBalancer(int port, const std::string& s1_ip, int s1_port, 
                 const std::string& s2_ip, int s2_port) 
        : PORT(port), server1_ip(s1_ip), server1_port(s1_port),
          server2_ip(s2_ip), server2_port(s2_port), current_server(0) {
        setupServer();
    }
    
    /**
     * @brief Destructor
     * 
     * Closes the server socket.
     */
    ~LoadBalancer() {
        close(server_fd);
    }
    
    /**
     * @brief Sets up the server socket
     * 
     * Creates and configures the server socket, binds it to the port,
     * and starts listening for connections.
     */
    void setupServer() {
        // Create socket file descriptor
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            throw std::runtime_error("Socket creation failed");
        }
        
        // Set socket options to reuse address
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            throw std::runtime_error("Socket option setting failed");
        }
        
        // Configure server address
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);
        
        // Bind socket to address
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            throw std::runtime_error("Socket binding failed");
        }
        
        // Start listening with increased backlog
        if (listen(server_fd, 1024) < 0) {
            throw std::runtime_error("Listening failed");
        }
        
        std::cout << "Load Balancer started on port " << PORT << std::endl;
    }
    
    /**
     * @brief Connects to a backend server
     * @return Socket file descriptor for the backend connection, or -1 on failure
     * 
     * Selects a backend server using round-robin and establishes a connection to it.
     */
    int connectToBackend() {
        // Round-robin selection of backend server
        std::string server_ip;
        int server_port;
        
        if (current_server == 0) {
            server_ip = server1_ip;
            server_port = server1_port;
            current_server = 1;
        } else {
            server_ip = server2_ip;
            server_port = server2_port;
            current_server = 0;
        }
        
        // Create socket for backend connection
        int backend_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (backend_socket < 0) {
            std::cerr << "Backend socket creation failed" << std::endl;
            return -1;
        }
        
        // Set up backend server address
        struct sockaddr_in backend_addr;
        backend_addr.sin_family = AF_INET;
        backend_addr.sin_port = htons(server_port);
        
        // Convert IP address to binary form
        if (inet_pton(AF_INET, server_ip.c_str(), &backend_addr.sin_addr) <= 0) {
            std::cerr << "Invalid backend address" << std::endl;
            close(backend_socket);
            return -1;
        }
        
        // Connect to backend server
        if (connect(backend_socket, (struct sockaddr*)&backend_addr, sizeof(backend_addr)) < 0) {
            std::cerr << "Connection to backend server failed" << std::endl;
            close(backend_socket);
            return -1;
        }
        
        return backend_socket;
    }
    
    /**
     * @brief Handles communication between a client and a backend server
     * @param client_socket Socket file descriptor for the client connection
     * 
     * Establishes a connection to a backend server and forwards data
     * between the client and the selected backend server.
     */
    void handleClient(int client_socket) {
        // Connect to a backend server
        int backend_socket = connectToBackend();
        if (backend_socket < 0) {
            close(client_socket);
            return;
        }
        
        // Set up poll for both sockets
        std::vector<pollfd> poll_fds(2);
        poll_fds[0].fd = client_socket;
        poll_fds[0].events = POLLIN;
        poll_fds[1].fd = backend_socket;
        poll_fds[1].events = POLLIN;
        
        char buffer[4096];
        bool client_closed = false;
        bool backend_closed = false;
        
        // Forward data between client and backend
        while (!client_closed && !backend_closed) {
            int poll_count = poll(poll_fds.data(), 2, -1);
            if (poll_count < 0) {
                std::cerr << "Poll error" << std::endl;
                break;
            }
            
            // Check for client data
            if (poll_fds[0].revents & POLLIN) {
                int bytes_read = read(client_socket, buffer, sizeof(buffer));
                if (bytes_read <= 0) {
                    client_closed = true;
                } else {
                    // Forward client data to backend
                    write(backend_socket, buffer, bytes_read);
                }
            }
            
            // Check for backend data
            if (poll_fds[1].revents & POLLIN) {
                int bytes_read = read(backend_socket, buffer, sizeof(buffer));
                if (bytes_read <= 0) {
                    backend_closed = true;
                } else {
                    // Forward backend data to client
                    write(client_socket, buffer, bytes_read);
                }
            }
        }
        
        // Clean up
        close(client_socket);
        close(backend_socket);
    }
    
    /**
     * @brief Starts the load balancer
     * 
     * Main event loop that accepts client connections and handles them
     * by creating a new process for each connection.
     */
    void start() {
        std::vector<pollfd> poll_fds(MAX_CLIENTS + 1);
        poll_fds[0].fd = server_fd;
        poll_fds[0].events = POLLIN;
        
        int num_fds = 1;
        while (true) {
            int poll_count = poll(poll_fds.data(), num_fds, -1);
            
            if (poll_count < 0) {
                std::cerr << "Poll error" << std::endl;
                continue;
            }
            
            // Handle new connection
            if (poll_fds[0].revents & POLLIN) {
                sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                
                if (client_socket < 0) {
                    std::cerr << "Accept failed" << std::endl;
                    continue;
                }
                
                // Handle client in a new thread or process
                // For simplicity, we'll fork a new process
                pid_t pid = fork();
                if (pid == 0) {
                    // Child process
                    close(server_fd);
                    handleClient(client_socket);
                    exit(0);
                } else {
                    // Parent process
                    close(client_socket);
                }
            }
        }
    }
};

/**
 * @brief Main function
 * @param argc Number of command-line arguments
 * @param argv Array of command-line arguments
 * @return Exit code (0 for success, 1 for failure)
 * 
 * Parses command-line arguments and starts the load balancer with
 * the specified configuration.
 */
int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <load_balancer_port> <server1_ip> <server1_port> <server2_ip> <server2_port>" << std::endl;
        return 1;
    }
    
    int lb_port = std::stoi(argv[1]);
    std::string server1_ip = argv[2];
    int server1_port = std::stoi(argv[3]);
    std::string server2_ip = argv[4];
    int server2_port = std::stoi(argv[5]);
    
    try {
        LoadBalancer lb(lb_port, server1_ip, server1_port, server2_ip, server2_port);
        lb.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

