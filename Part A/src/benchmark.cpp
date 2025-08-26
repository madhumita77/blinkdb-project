/**
 * @file benchmark.cpp
 * @brief Performance benchmarking for BlinkDB
 * @author Madhumita
 * @date 2025-03-31
 */

#include "blinkdb.h"
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>

/**
 * @brief Benchmarks read-heavy operations
 * @param db Reference to the BlinkDB instance
 * 
 * Performs 1 million write operations followed by 1 million read operations
 * and measures the time taken for the read operations.
 */
void benchmarkReadHeavy(BlinkDB& db) {
    std::cout << "Read Heavy Benchmark\n";
    
    for (int i = 0; i < 1000000; ++i) {
        std::string key = "key" + std::to_string(i);
        db.set(key, "value" + std::to_string(i));
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000000; ++i) {
        std::string key = "key" + std::to_string(i);
        db.get(key);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
}

/**
 * @brief Benchmarks write-heavy operations
 * @param db Reference to the BlinkDB instance
 * 
 * Performs and measures the time for 1 million write operations.
 */
void benchmarkWriteHeavy(BlinkDB& db) {
    std::cout << "Write Heavy Benchmark\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000000; ++i) {
        std::string key = "key" + std::to_string(i);
        db.set(key, "value" + std::to_string(i));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
}

/**
 * @brief Benchmarks mixed read/write operations
 * @param db Reference to the BlinkDB instance
 * 
 * Performs 500k initial writes, then measures the time for 500k reads
 * followed by 500k writes.
 */
void benchmarkMixed(BlinkDB& db) {
    std::cout << "Mixed Benchmark\n";
    
    for (int i = 0; i < 500000; ++i) {
        std::string key = "key" + std::to_string(i);
        db.set(key, "value" + std::to_string(i));
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 500000; ++i) {
        std::string key = "key" + std::to_string(i);
        db.get(key);
    }
    
    for (int i = 0; i < 500000; ++i) {
        std::string key = "key" + std::to_string(i);
        db.set(key, "new_value" + std::to_string(i));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
}

/**
 * @brief Main function for running benchmarks
 * @return Exit code
 * 
 * Creates a BlinkDB instance and runs all three benchmarks in sequence,
 * clearing the persistence file between each benchmark.
 */
int main() {
    BlinkDB db;
    db.clearPersistenceFile();
    benchmarkReadHeavy(db);
    db.clearPersistenceFile(); // Clear data for next benchmark
    benchmarkWriteHeavy(db);
    db.clearPersistenceFile(); // Clear data for next benchmark
    benchmarkMixed(db);
    db.clearPersistenceFile();
    return 0;
}

