/**
 * @file blinkdb.h
 * @brief Header file for the BlinkDB in-memory database with LRU caching
 * @author Madhumita
 * @date 2025-03-31
 */

#ifndef BLINKDB_H
#define BLINKDB_H

#include <unordered_map>
#include <list>
#include <string>
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <iostream>
#include <future>
#include <exception>
#include <cstdio>

#define VALUE_SIZE 256
#define MAX_CAPACITY 10000
#define FLUSH_FILE "flush_data.txt"
#define COMPACTION_THRESHOLD 1000

/**
 * @class BlinkDB
 * @brief An in-memory key-value database with LRU caching and disk persistence
 *
 * BlinkDB implements a simple key-value store with an LRU (Least Recently Used)
 * eviction policy. It provides persistence by periodically flushing data to disk
 * and can restore evicted keys from disk when requested.
 */
class BlinkDB {
private:
    /**
     * @brief Main storage for key-value pairs
     */
    std::unordered_map<std::string, std::string> store;
    
    /**
     * @brief List maintaining LRU order of keys
     */
    std::list<std::string> lru_keys;
    
    /**
     * @brief Map for quick access to keys' positions in the LRU list
     */
    std::unordered_map<std::string, std::list<std::string>::iterator> lru_map;
    
    /**
     * @brief Set of keys that have been evicted from memory but exist on disk
     */
    std::unordered_map<std::string, bool> evicted_keys;
    
    /**
     * @brief Mutex for thread-safe access to the database
     */
    mutable std::shared_mutex db_mutex;
    
    /**
     * @brief Maximum number of items to keep in memory
     */
    const size_t max_cache_size = MAX_CAPACITY;
    
    /**
     * @brief Path to the persistence file
     */
    const std::string persistence_file = FLUSH_FILE;
    
    /**
     * @brief Flag indicating whether data has been modified since last flush
     */
    bool dirty = false;
    
    /**
     * @brief Loads data from persistence file into memory
     */
    void loadFromFile();
    
    /**
     * @brief Updates the LRU status of a key
     * @param key The key to update in the LRU cache
     */
    void updateLRU(const std::string& key);
    
    /**
     * @brief Restores an evicted key from disk
     * @param key The key to restore
     */
    void restoreFromDisk(const std::string& key);

public:
    /**
     * @brief Constructor
     * 
     * Initializes the database and starts the background flush thread
     */
    BlinkDB();
    
    /**
     * @brief Destructor
     * 
     * Ensures any pending changes are persisted to disk
     */
    ~BlinkDB();
    
    /**
     * @brief Sets a key-value pair in the database
     * @param key The key to set
     * @param value The value to associate with the key
     */
    void set(const std::string& key, const std::string& value);
    
    /**
     * @brief Retrieves a value by key
     * @param key The key to look up
     * @return The value associated with the key, or "NULL" if not found
     */
    std::string get(const std::string& key);
    
    /**
     * @brief Deletes a key-value pair from the database
     * @param key The key to delete
     * @return true if the key was found and deleted, false otherwise
     */
    bool del(const std::string& key);
    
    /**
     * @brief Writes all in-memory data to disk
     */
    void persistToFile();
    
    /**
     * @brief Deletes the persistence file
     */
    void clearPersistenceFile();
    
    /**
     * @brief Background thread function that periodically flushes data to disk
     */
    void flushToDiskPeriodically();
    
    /**
     * @brief Asynchronously flushes data to disk
     */
    void flushToDiskAsync();
};

#endif // BLINKDB_H

