/**
 * @file blink_server.h
 * @brief Header file for the BlinkServer class implementing a Redis-like server
 * @author Madhumita
 * @date 2025-03-31
 */
 
#ifndef BLINK_SERVER_H
#define __BLINK_SERVER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <cstring>
#include <memory>
#include <algorithm>
#include <sys/epoll.h>
#include "blinkdb.h"

/**
 * @class BlinkServer
 * @brief Implements a Redis-compatible server using the RESP-2 protocol
 *
 * This class provides a server that listens for client connections and processes
 * Redis-compatible commands using the RESP-2 protocol. It uses epoll for efficient
 * handling of multiple client connections.
 */
 
class BlinkServer {
private:
    /**
     * @brief Port number the server listens on
     */
    static const int PORT = 9001;
    
    /**
     * @brief Maximum number of simultaneous client connections
     */
    static const int MAX_CLIENTS = 1500;
    
    /**
     * @brief Socket file descriptor for the server
     */
    int server_fd;
    
    /**
     * @brief Server address structure
     */
    struct sockaddr_in address;
    
    /**
     * @brief Database instance for storing key-value pairs
     */
    std::unique_ptr<BlinkDB> database;

    /**
     * @brief Encodes a simple string in RESP-2 format
     * @param msg The string to encode
     * @return The RESP-2 encoded simple string
     */
    std::string encodeSimpleString(const std::string& msg);
    
    /**
     * @brief Encodes a bulk string in RESP-2 format
     * @param msg The string to encode
     * @return The RESP-2 encoded bulk string
     */
    std::string encodeBulkString(const std::string& msg);
    
    /**
     * @brief Encodes an integer in RESP-2 format
     * @param value The integer to encode
     * @return The RESP-2 encoded integer
     */
    std::string encodeInteger(int value);
    
    /**
     * @brief Encodes an error message in RESP-2 format
     * @param msg The error message to encode
     * @return The RESP-2 encoded error message
     */
    std::string encodeError(const std::string& msg);
    
    /**
     * @brief Decodes a RESP-2 command from raw input
     * @param raw_input The raw input string to decode
     * @return Vector of command arguments
     */
    std::vector<std::string> decodeCommand(const std::string& raw_input);

     /**
     * @brief Handles a decoded command
     * @param command Vector of command arguments
     * @return RESP-2 encoded response
     */
    std::string handleCommand(const std::vector<std::string>& command);
    
    /**
     * @brief Processes a SET command
     * @param args Command arguments
     * @return RESP-2 encoded response
     */
    std::string processSet(const std::vector<std::string>& args);
    
    /**
     * @brief Processes a GET command
     * @param args Command arguments
     * @return RESP-2 encoded response
     */
    std::string processGet(const std::vector<std::string>& args);
    
    /**
     * @brief Processes a DEL command
     * @param args Command arguments
     * @return RESP-2 encoded response
     */
    std::string processDel(const std::vector<std::string>& args);

    /**
     * @brief Sets up the server socket
     * 
     * Creates and configures the server socket, binds it to the port,
     * and starts listening for connections.
     */
    void setupServer();
    
    /**
     * @brief Handles client connections using epoll
     * 
     * Main event loop that accepts new connections and processes
     * client requests using epoll for efficient I/O multiplexing.
     */
    void handleClientConnections();
    
    /**
     * @brief Handles a read event from a client
     * @param client_socket The client socket file descriptor
     * 
     * Reads data from the client, processes the command, and sends the response.
     */
    void handleClientRead(int client_socket);

public:
    /**
     * @brief Constructor
     * 
     * Initializes the database and sets up the server socket.
     */
    BlinkServer();
    
    /**
     * @brief Destructor
     * 
     * Closes the server socket.
     */
    ~BlinkServer();
    
    /**
     * @brief Starts the server
     * 
     * Begins listening for and handling client connections.
     */
    void start();
};

#endif // BLINK_SERVER_H
