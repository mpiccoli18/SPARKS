/**
 * @file SocketModule.cpp
 * @brief SocketModule implementation
 * 
 * This file holds the SocketModules class implementation.
 * 
 */

#include "SocketModule.hpp"

/// @brief Constructor: Initializes socket
SocketModule::SocketModule() : socket_fd(-1), connection_fd(-1) {}

/// @brief Initiates a client connection
bool SocketModule::initiateConnection(const std::string& ip, int port) {
    
    // Create a socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Socket creation failed");
        return false;
    }

    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    struct timeval timeout;      
    timeout.tv_sec = TIMEOUT_VALUE;  // Timeout after 5 seconds
    timeout.tv_usec = 0; 

    // Set the timeout
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) <= 0) {
        perror("Invalid address");
        return false;
    }

    // Connect to the server
    if (connect(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Connection failed");
        return false;
    }

    connection_fd = socket_fd;
    return true;
}

/// @brief Waits for a client to connect (acts as a server)
bool SocketModule::waitForConnection(int port) {
    
    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Socket creation failed");
        return false;
    }

    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    struct timeval timeout;      
    timeout.tv_sec = TIMEOUT_VALUE;  // Timeout after 5 seconds
    timeout.tv_usec = 0; 

    // setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    // Setup server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    // Bind the socket
    if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return false;
    }

    // Start listening
    if (listen(socket_fd, 3) < 0) {
        perror("Listen failed");
        return false;
    }

    // std::cout << "Waiting for a connection on port " << port << "...\n";

    socklen_t addr_len = sizeof(address);
    connection_fd = accept(socket_fd, (struct sockaddr*)&address, &addr_len);
    if (connection_fd < 0) {
        perror("Accept failed");
        return false;
    }

    // std::cout << "Client connected!\n";

    // Set the timeout
    setsockopt(connection_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    setsockopt(connection_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    return true;
}

/// @brief Send a msgPack message over the socket
/// @param msgPack The message to send
void SocketModule::sendMsg(const std::unordered_map<std::string, std::string> &msgPack) {
    // Serialize the map using msgpack
    if(this->isOpen() == false) {
        std::cerr << "Error: Connection is not open!" << std::endl;
        return;
    }
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, msgPack);
    send(this->connection_fd, sbuf.data(), sbuf.size(), 0);
}

/// @brief Receive a msgPack message on the socket
/// @param existingMap Pointer to an existing map to store the received message
/// @return The received message as a string
void SocketModule::receiveMsg(std::unordered_map<std::string, std::string> &msg) {
    char buffer[1024] = {0};
    msgpack::object_handle msgpack_obj;
    msgpack::object obj = msgpack_obj.get();
    msgpack::unpacker pac;
    std::string key, value;
    int bytesReceived;
    if(this->isOpen() == false) {
        std::cerr << "Error: Connection is not open!" << std::endl;
        return;
    }
    if (pac.next(msgpack_obj)) { // This is true only if there is a complete parsed message in the unpacker 'pac'
        return;
    }

    bytesReceived = read(this->connection_fd, buffer, sizeof(buffer));
    if (bytesReceived <= 0) {
        if (bytesReceived == 0) {
            std::cerr << "Connection closed by peer." << std::endl;
        } else {
            perror("Receive failed");
        }
        return;
    }
    // Deserialize the map
    msgpack::unpack(msgpack_obj, buffer, sizeof(buffer));
    obj = msgpack_obj.get();
    if(bytesReceived > 0) {
        PROD_ONLY({std::cout << "Received " << bytesReceived << " bytes." << std::endl;});
    }

    for(uint32_t i = 0; i< obj.via.map.size; ++i){
        const msgpack::object_kv& kv = obj.via.map.ptr[i];
        
        kv.key.convert(key);
        kv.val.convert(value);
        msg.emplace(std::move(key), std::move(value));
    }

    return;
}

/// @brief Close the connection
void SocketModule::closeConnection() {
    if (connection_fd != -1) close(connection_fd);
    if (socket_fd != -1) close(socket_fd);
}

/// @brief Destructor ensures the connection is closed
SocketModule::~SocketModule() {
    closeConnection();
}

/// @brief Check if the socket is open
bool SocketModule::isOpen() const {
    return (socket_fd != -1 && connection_fd != -1);
}

/// @brief Get the socket file descriptor
int SocketModule::getSocketFd() const {
    return socket_fd;
}

/// @brief Get the connection file descriptor
int SocketModule::getConnectionFd() const {
    return connection_fd;
}
