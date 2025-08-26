/**
 * @file main.cpp
 * @brief Main entry point for the BlinkServer application
 * @author Madhumita
 * @date 2025-03-31
 * 
 * This file contains the main function that initializes and starts
 * the BlinkServer, handling any exceptions that may occur during startup.
 */
 
#include "blink_server.h"

/**
 * @brief Main function
 * @return Exit code (0 for success, 1 for failure)
 * 
 * Creates and starts a BlinkServer instance, catching and reporting any
 * exceptions that occur during server startup.
 */
int main() {
    try {
        BlinkServer server;
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Server startup failed: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
