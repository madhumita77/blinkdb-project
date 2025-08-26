/**
 * @file main.cpp
 * @brief REPL interface for BlinkDB
 * @author Madhumita
 * @date 2025-03-31
 * 
 * Compilation: g++ -std=c++17 -lpthread blinkdb.cpp benchmark.cpp -o benchmark
 * Execution: ./benchmark
 */

#include "blinkdb.h"
#include <iostream>
#include <string>
#include <sstream>

/**
 * @brief Main function implementing a REPL for BlinkDB
 * @return Exit code
 * 
 * Provides a command-line interface for interacting with BlinkDB.
 * Supported commands:
 * - SET <key> <value>: Sets a key-value pair
 * - GET <key>: Retrieves a value by key
 * - DEL <key>: Deletes a key-value pair
 * - EXIT/QUIT: Exits the program
 */
int main() {
    BlinkDB db;
    std::string command;
    
    std::cout << "BlinkDB REPL\n";
    std::cout << "Commands: SET <key> <value>, GET <key>, DEL <key>\n";
    
    while (true) {
        std::cout << "User> ";
        std::getline(std::cin, command);
        
        if (command.empty()) continue;
        
        std::istringstream iss(command);
        std::string operation, key;
        iss >> operation;
        
        if (operation == "SET") {
            iss >> key;
            std::string value;
            
            if (!(iss >> std::ws)) { // Check if there's a value after key
                std::cout << "ERROR: Invalid command. Value must be provided.\n";
                continue;
            }
            
            std::getline(iss, value); // Read rest of the line after key
            if (value.empty()) {
                std::cout << "ERROR: Value cannot be empty.\n";
                continue;
            }
            
            db.set(key, value);
        }
        else if (operation == "GET") {
            iss >> key;
            std::string result = db.get(key);
            
            if (result.empty() || result == "NULL") {
                std::cout << "NULL\n";
            } else {
                std::cout << result << "\n";
            }
        }
        else if (operation == "DEL") {
            iss >> key;
            bool success = db.del(key);
            
            if (success) {
                std::cout << "OK\n";
            } else {
                std::cout << "Does not exist.\n";
            }
        }
        else if (operation == "EXIT" || operation == "QUIT") {
            break;
        }
        else {
            std::cout << "ERROR: Invalid command\n";
        }
    }
    
    return 0;
}

