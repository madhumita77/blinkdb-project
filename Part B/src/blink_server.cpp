/**
 * @file blink_server.cpp
 * @brief Implementation of the BlinkServer class
 * @author Madhumita
 * @date 2025-03-31
 */
 
#include "blink_server.h"

/**
 * @brief Constructor implementation
 * 
 * Initializes the database and sets up the server socket.
 */
BlinkServer::BlinkServer() : database(std::make_unique<BlinkDB>()) {
    setupServer();
}

/**
 * @brief Destructor implementation
 * 
 * Closes the server socket.
 */
BlinkServer::~BlinkServer() {
    close(server_fd);
}

/**
 * @brief Sets up the server socket
 * 
 * Creates and configures the server socket, binds it to the port,
 * and starts listening for connections.
 */
void BlinkServer::setupServer() {
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
    if (listen(server_fd, 1024) < 0) { // Increased backlog to 1024
        throw std::runtime_error("Listening failed");
    }
}

/**
 * @brief Starts the server
 * 
 * Begins listening for and handling client connections.
 */
void BlinkServer::start() {
    std::cout << "BLINK DB Server started on port " << PORT << std::endl;
    handleClientConnections();
}

/**
 * @brief Handles client connections using epoll
 * 
 * Main event loop that accepts new connections and processes
 * client requests using epoll for efficient I/O multiplexing.
 */
void BlinkServer::handleClientConnections() {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        std::cerr << "Epoll creation failed" << std::endl;
        return;
    }

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
        std::cerr << "Epoll control failed" << std::endl;
        return;
    }

    std::vector<epoll_event> events(MAX_CLIENTS + 1);

    while (true) {
        int num_events = epoll_wait(epoll_fd, events.data(), MAX_CLIENTS + 1, -1);
        if (num_events < 0) {
            std::cerr << "Epoll wait failed" << std::endl;
            continue;
        }

        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == server_fd) {
                // Handle new connection
                sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

                if (client_socket < 0) {
                    std::cerr << "Accept failed" << std::endl;
                    continue;
                }

                // Add to epoll
                event.events = EPOLLIN;
                event.data.fd = client_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) < 0) {
                    std::cerr << "Epoll control failed" << std::endl;
                    close(client_socket);
                    continue;
                }
            } else {
                // Handle client data
                handleClientRead(events[i].data.fd);
            }
        }
    }
}

/**
 * @brief Handles a read event from a client
 * @param client_socket The client socket file descriptor
 * 
 * Reads data from the client, processes the command, and sends the response.
 */
void BlinkServer::handleClientRead(int client_socket) {
    char buffer[1024] = {0};
    int bytes_read = read(client_socket, buffer, sizeof(buffer));

    if (bytes_read <= 0) {
        close(client_socket); // Close connection if read fails or client disconnects
        return;
    }

    std::string raw_input(buffer, bytes_read);
    std::vector<std::string> command = decodeCommand(raw_input);

    if (command.empty()) {
        std::string error = encodeError("Invalid Command");
       	write(client_socket, error.c_str(), error.length());
        return;
    }

    std::string response = handleCommand(command);
    write(client_socket, response.c_str(), response.length());
}

/**
 * @brief Decodes a RESP-2 command from raw input
 * @param raw_input The raw input string to decode
 * @return Vector of command arguments
 * 
 * Parses RESP-2 protocol formatted commands into a vector of strings.
 */
std::vector<std::string> BlinkServer::decodeCommand(const std::string& raw_input) {
    std::vector<std::string> parsed_command;
    size_t index = 0;

    if (raw_input.empty() || raw_input[index] != '*') return parsed_command;

    index++;

    size_t next = raw_input.find("\r\n", index);

    if (next == std::string::npos) return parsed_command;

    int arg_count = std::stoi(raw_input.substr(index, next - index));

    index = next + 2;

    for (int i = 0; i < arg_count; i++) {
        if (index >= raw_input.size() || raw_input[index] != '$') return parsed_command;

        index++; 
        next = raw_input.find("\r\n", index);
        if (next == std::string::npos) return parsed_command;
        int len = std::stoi(raw_input.substr(index, next - index));
        index = next + 2; 

        if (index + len > raw_input.size()) return parsed_command;
        parsed_command.push_back(raw_input.substr(index, len));
        index += len + 2;
    }

    return parsed_command;
}

/**
 * @brief Handles a decoded command
 * @param command Vector of command arguments
 * @return RESP-2 encoded response
 * 
 * Routes the command to the appropriate handler based on the command type.
 */
std::string BlinkServer::handleCommand(const std::vector<std::string>& command) {
    if (command.empty()) {
        return encodeError("Empty command");
    }

    std::string cmd = command[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    if (cmd == "SET" && command.size() == 3) {
        return processSet(command);
    } else if (cmd == "GET" && command.size() == 2) {
        return processGet(command);
    } else if (cmd == "DEL" && command.size() == 2) {
        return processDel(command);
    } else if (cmd == "CONFIG") {
        return "*0\r\n";
    } else {
        return encodeError("Unknown command");
    }
}

/**
 * @brief Processes a SET command
 * @param args Command arguments
 * @return RESP-2 encoded response
 * 
 * Sets a key-value pair in the database.
 */
std::string BlinkServer::processSet(const std::vector<std::string>& args) {
    //std::cout << "DEBUG: SET key=" << args[1] << " value=" << args[2] << std::endl;
    database->set(args[1], args[2]);
    return encodeSimpleString("OK");
}

/**
 * @brief Processes a GET command
 * @param args Command arguments
 * @return RESP-2 encoded response
 * 
 * Retrieves a value by key from the database.
 */
std::string BlinkServer::processGet(const std::vector<std::string>& args) {
    std::string value = database->get(args[1]);
    //std::cout << "DEBUG: GET key=" << args[1] << " value=" << value << std::endl;
    return (value == "NULL") ? encodeBulkString("") : encodeBulkString(value);
}

/**
 * @brief Processes a DEL command
 * @param args Command arguments
 * @return RESP-2 encoded response
 * 
 * Deletes a key-value pair from the database.
 */
std::string BlinkServer::processDel(const std::vector<std::string>& args) {
    bool deleted = database->del(args[1]);
    return encodeInteger(deleted ? 1 : 0);
}

/**
 * @brief Encodes a simple string in RESP-2 format
 * @param msg The string to encode
 * @return The RESP-2 encoded simple string
 */
std::string BlinkServer::encodeSimpleString(const std::string& msg) {
    return "+" + msg + "\r\n";
}

/**
 * @brief Encodes a bulk string in RESP-2 format
 * @param msg The string to encode
 * @return The RESP-2 encoded bulk string
 */
std::string BlinkServer::encodeBulkString(const std::string& msg) {
    if (msg.empty()) {
        return "$-1\r\n";  // Null bulk string
    }
    return "$" + std::to_string(msg.length()) + "\r\n" + msg + "\r\n";
}

/**
 * @brief Encodes an integer in RESP-2 format
 * @param value The integer to encode
 * @return The RESP-2 encoded integer
 */
std::string BlinkServer::encodeInteger(int value) {
    return ":" + std::to_string(value) + "\r\n";
}

/**
 * @brief Encodes an error message in RESP-2 format
 * @param msg The error message to encode
 * @return The RESP-2 encoded error message
 */
std::string BlinkServer::encodeError(const std::string& msg) {
    return "-ERR " + msg + "\r\n";
}
