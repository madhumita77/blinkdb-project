/**
 * @file blinkdb.cpp
 * @brief Implementation of the BlinkDB class
 * @author Madhumita
 * @date 2025-03-31
 */

#include "blinkdb.h"

/**
 * @brief Removes the persistence file from disk
 */
void BlinkDB::clearPersistenceFile() {
    std::remove(persistence_file.c_str());
}

/**
 * @brief Constructor implementation
 * 
 * Loads existing data from disk and starts the background flush thread
 */
BlinkDB::BlinkDB() {
    //clearPersistenceFile(); // Commenting to keep all the data stored even when the server is closed.
    loadFromFile();
    
    // Start periodic flushing thread
    std::thread flush_thread(&BlinkDB::flushToDiskPeriodically, this);
    flush_thread.detach();
}

/**
 * @brief Destructor implementation
 * 
 * Ensures any unsaved changes are written to disk
 */
BlinkDB::~BlinkDB() {
    if (dirty) {
        persistToFile();
    }
}

/**
 * @brief Sets a key-value pair in the database
 * @param key The key to set
 * @param value The value to associate with the key
 */
void BlinkDB::set(const std::string& key, const std::string& value) {
    std::unique_lock lock(db_mutex);
    
    // Update LRU
    updateLRU(key);
    
    // Store value
    store[key] = value;
    dirty = true;
}

/**
 * @brief Retrieves a value by key
 * @param key The key to look up
 * @return The value associated with the key, or "NULL" if not found
 */
std::string BlinkDB::get(const std::string& key) {
    std::shared_lock read_lock(db_mutex);
    
    auto it = store.find(key);
    if (it == store.end()) {
        // Check if key is evicted
        if (evicted_keys.find(key) != evicted_keys.end()) {
            // Restore from disk
            restoreFromDisk(key);
            it = store.find(key);
        }
        
        if (it == store.end()) {
            return "NULL";
        }
    }
    
    // Convert to unique lock to update LRU
    read_lock.unlock();
    std::unique_lock write_lock(db_mutex);
    
    // Update LRU cache
    updateLRU(key);
    return it->second;
}

/**
 * @brief Restores an evicted key from disk
 * @param key The key to restore
 */
void BlinkDB::restoreFromDisk(const std::string& key) {
    std::ifstream in(persistence_file);
    if (in) {
        std::string line_key, value;
        while (std::getline(in, line_key, '\t') && std::getline(in, value)) {
            if (line_key == key) {
                store[key] = value;
                lru_keys.push_front(key);
                lru_map[key] = lru_keys.begin();
                evicted_keys.erase(key); // Remove from evicted keys
                break;
            }
        }
    }
}

/**
 * @brief Deletes a key-value pair from the database
 * @param key The key to delete
 * @return true if the key was found and deleted, false otherwise
 */
bool BlinkDB::del(const std::string& key) {
    std::unique_lock lock(db_mutex);
    
    auto it = store.find(key);
    if (it == store.end()) {
        return false;
    }
    
    // Remove from LRU cache
    if (lru_map.find(key) != lru_map.end()) {
        lru_keys.erase(lru_map[key]);
        lru_map.erase(key);
    }
    
    // Remove from store
    store.erase(it);
    dirty = true;
    return true;
}

/**
 * @brief Updates the LRU status of a key
 * @param key The key to update in the LRU cache
 * 
 * Moves the key to the front of the LRU list and handles eviction if needed
 */
void BlinkDB::updateLRU(const std::string& key) {
    // If key exists in LRU, remove it
    if (lru_map.find(key) != lru_map.end()) {
        lru_keys.erase(lru_map[key]);
        lru_map.erase(key);
    }
    
    // Add key to front of LRU list
    lru_keys.push_front(key);
    lru_map[key] = lru_keys.begin();
    
    // Evict if needed
    if (lru_keys.size() > max_cache_size) {
        std::string evict_key = lru_keys.back();
        
        // Before evicting, save the key to a map if it exists in store
        if (store.find(evict_key) != store.end()) {
            evicted_keys[evict_key] = true; // Mark as evicted
        }
        
        store.erase(evict_key);
        lru_map.erase(evict_key);
        lru_keys.pop_back();
        dirty = true;
    }
}

/**
 * @brief Writes all in-memory data to disk
 */
void BlinkDB::persistToFile() {
    std::ofstream out(persistence_file);
    if (out) {
        for (const auto& [key, value] : store) {
            out << key << "\t" << value << "\n";
        }
    }
    dirty = false;
}

/**
 * @brief Loads data from persistence file into memory
 */
void BlinkDB::loadFromFile() {
    std::ifstream in(persistence_file);
    if (in) {
        std::string key, value;
        while (std::getline(in, key, '\t') && std::getline(in, value)) {
            store[key] = value;
            lru_keys.push_front(key);
            lru_map[key] = lru_keys.begin();
        }
    } else {
        // File does not exist or is empty, do nothing
    }
}

/**
 * @brief Background thread function that periodically flushes data to disk
 */
void BlinkDB::flushToDiskPeriodically() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10)); // Flush every 10 seconds
        if (dirty) {
            persistToFile();
        }
    }
}

/**
 * @brief Asynchronously flushes data to disk
 */
void BlinkDB::flushToDiskAsync() {
    auto future = std::async(std::launch::async, &BlinkDB::persistToFile, this);
    try {
        future.get(); // Wait for the future to complete
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

