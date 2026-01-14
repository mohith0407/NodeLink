# C++ BitTorrent Client

A lightweight, multi-threaded BitTorrent client written from scratch in C++. This project demonstrates a deep implementation of the BitTorrent Protocol (BTP/1.0) without relying on high-level networking libraries.

The primary goal of this project is to explore low-level system engineering concepts, including **TCP/IP socket programming**, **non-blocking I/O (epoll)**, and **binary data parsing**.

> **Note:** The only external library used is **OpenSSL** (for computing SHA-1 hashes required by the protocol). All networking and protocol logic is custom-built.

---

## Features
* **Zero-Dependency Networking:** Uses raw Linux system calls (`socket`, `connect`, `send`, `recv`) instead of libraries like Boost.Asio.
* **Bencoding Parser:** Custom implementation to decode `.torrent` metadata files.
* **Event-Driven I/O:** Utilizes Linux `epoll` for handling multiple peer connections efficiently.
* **Piece Management:** Implements "rarest-first" logic (basic) and managing file assembly from out-of-order packets.

---

## System Architecture & Data Flow
How do we go from a static `.torrent` file to a downloaded movie? Here is the lifecycle of a download in this system:

### 1. The Parser (Bencoding Analysis)
* **Input:** The user provides a `.torrent` file (e.g., `sintel.torrent`).
* **Action:** The system reads this file using a custom **Bencoding Parser**. It decodes the hierarchical dictionary structure to extract key metadata:
    * **Announce URL:** The address of the Tracker server.
    * **Info Hash:** The unique SHA-1 fingerprint of the file we want.
    * **Piece Length:** The size of each data chunk.

### 2. Tracker Communication
* **Action:** The client sends an HTTP GET request to the **Announce URL** containing the `info_hash` and our client ID.
* **Result:** The Tracker responds with a compact binary list of IP addresses and Ports. These are other computers (Peers) that have the file.

### 3. Peer Connection (The Handshake)
* **Action:** The client opens non-blocking TCP sockets to these IP addresses.
* **Protocol:** Before exchanging data, we perform the **BitTorrent Handshake**:
    * Send: `19:BitTorrent protocol` + `Info Hash` + `Peer ID`.
    * Receive: If the peer recognizes the hash, they reply with the same handshake.
* **Status:** The connection state moves from `CONNECTING` to `HANDSHAKE_SUCCESS`.

### 4. The Download Loop (State Machine)
Once connected, the "Reactor" loop begins:
1.  **Bitfield Exchange:** The peer tells us which pieces they have (e.g., "I have pieces 1, 5, and 10").
2.  **Interested/Unchoke:** We tell the peer "I am interested," and wait for them to "Unchoke" us (allow us to request data).
3.  **Requesting Blocks:** We request specific blocks of data (16KB chunks) from the peer.
4.  **Assembly:** As raw bytes arrive, they are verified against the SHA-1 hash and written to the correct offset in the file on disk.

---

## Build & Run Instructions

This project is designed for Linux environments (Ubuntu/WSL).

### Prerequisites
* GCC Compiler (g++)
* CMake
* OpenSSL Development Libraries (`libssl-dev`)

### Installation
```bash
# 1. Clone the repository
git clone https://github.com/mohith0407/NodeLink.git
cd NodeLink

# 2. Create build directory
mkdir build && cd build

# 3. Compile the project
cmake ..
make BitTorrent
./source/BitTorrent <torrent_file>
```

## Testing

Run the following commands:

```bash
git clone https://github.com/google/googletest.git
mkdir build
cd build
cmake ..
make
```

After having built the project, the test binaries are located
in build/test/

## Usage
Run the client by passing a valid .torrent file as an argument. (Note: This client supports HTTP/UDP trackers. For best results, use standard single-file torrents like Debian or Sintel).

```bash
# Example: Downloading Debian Linux
./source/BitTorrent debian-12.9.0-amd64-netinst.iso.torrent
```
