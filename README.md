# 🚀 BlinkDB Project

![C++](https://img.shields.io/badge/language-C++17-blue.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)
![Status](https://img.shields.io/badge/status-completed-success.svg)

**BlinkDB** is a high-performance in-memory key-value database implemented in **C++**, inspired by Redis.  
It is built in two modular parts: a **storage engine** and a **network infrastructure**.

---

## ✨ Features

- ⚡ **In-Memory Storage Engine**
  - O(1) average `SET`, `GET`, `DEL` operations
  - LRU cache with eviction
  - Disk persistence with asynchronous flushing

- 🌐 **Network Infrastructure**
  - TCP server with **epoll** for I/O multiplexing
  - RESP2 protocol support (Redis compatible)
  - Load balancer with **round-robin distribution**
  - Benchmarked with `redis-benchmark`

- 🛠 **System Design**
  - Thread-safe operations with shared mutex
  - Optimized for write-heavy workloads
  - Modular architecture for reuse and extension

---

## 📂 Project Structure

```bash
blinkdb-project/
├── Part-A/
│   ├── docs/                        # Design doc + doxygen documentation
│   │   └── Design Document for BlinkDB Storage Engine.pdf
│   └── src/                         # Source code + Makefile
│       ├── blinkdb.cpp
│       ├── blinkdb.h
│       ├── benchmark.cpp
│       ├── repl.cpp
│       └── Makefile
│
├── Part-B/
│   ├── docs/                        # Design doc + doxygen documentation
│   │   └── Design Document for BlinkDB Network Infrastructure.pdf
│   └── src/                         # Source code + Makefile
│       ├── blink_server.cpp
│       ├── load_balancer.cpp
│       └── Makefile
│
├── reports/                         # Final reports
│   ├── DESIGN_LAB_PROJECT.pdf
│   ├── PartA_Report.pdf
│   └── PartB_Report.pdf
│
├── LICENSE
├── README.md
└── .gitignore
```

## ⚙️ Build & Run

### 🔹 Part A — Storage Engine

**Compile**
```bash
make clean   # remove old binaries and data files
make         # build benchmark + REPL
```

**Run Benchmarks**
```bash
make run_benchmark
```

**Run REPL**
```bash
make run_repl
```

### 🔹 Part B — Network Infrastructure

**Compile**
```bash
make clean
make
```

**Run Server (default port 9001)**
```bash
./blink_server
```

**Run Load Balancer**
```bash
./load_balancer <lb_port> <server1_ip> <server1_port> <server2_ip> <server2_port>
```

**Run Benchmark with Redis Tool**
```bash
redis-benchmark -h <ip> -p <port> -c <connections> -n <requests>
```

## 📊 Benchmarks

- **Storage Engine** tested on:
  - Read-heavy
  - Write-heavy
  - Mixed workloads  

- **Network Infrastructure** tested with `redis-benchmark`:
  - 10K, 100K, 1M concurrent requests  
  - 10, 100, 1000 parallel connections  

📂 Benchmark results are stored in:  

Part-B/result/

---

## 📖 Documentation

- 📑 **Design Docs** → available in `Part-A/docs/` and `Part-B/docs/`  
- 📄 **Doxygen Docs** → PDF + HTML generated from source  
- 📝 **Reports** → see `reports/` folder  

---

## 📜 License

This project is licensed under the [MIT License](LICENSE).

---

## 🙌 Acknowledgements

- Inspired by [Redis](https://redis.io) and its RESP2 protocol  
- Developed as part of **CS69202 – Design Laboratory Project**  

---

💡 *BlinkDB demonstrates how database internals (storage engines, caches, persistence) can be combined with networking (protocol parsing, multiplexing, load balancing) to create a modular, high-performance system.*
