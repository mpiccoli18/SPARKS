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

/**
 * @brief Receive a message on the msgPack format and return it in the unordered_map msg.  
 * 
 * @param msg 
 */
void SocketModule::receiveMsg(std::unordered_map<std::string, std::string> &msg){
    char buffer[1024] = {0};
    msgpack::object_handle msgpack_obj;

    if (pac.next(msgpack_obj)) { // This is true only if there is a complete parsed message in the unpacker 'pac'
        msgpack::object obj = msgpack_obj.get();

        if (obj.type != msgpack::type::MAP) {
            throw std::runtime_error("Expected a map");
        }

        for (uint32_t i = 0; i < obj.via.map.size; ++i) {
            const msgpack::object_kv& kv = obj.via.map.ptr[i];

            std::string key;
            std::string value;

            kv.key.convert(key);    
            kv.val.convert(value);   

            msg.emplace(std::move(key), std::move(value));  // Insert directly
        }
        return;
    }
    while(true)
    {
        int bytesReceived = read(this->connection_fd, buffer, sizeof(buffer));
        if (bytesReceived > 0) { 
            PROD_ONLY({std::cout << "Received " << bytesReceived << "bytes." << std::endl;});
            pac.reserve_buffer(bytesReceived);
            std::memcpy(pac.buffer(), buffer, bytesReceived);
            pac.buffer_consumed(bytesReceived);

            if (pac.next(msgpack_obj)) { // Check whether there is a complete message
                msgpack::object obj = msgpack_obj.get();    // Get the object

                if (obj.type != msgpack::type::MAP) {       // Verify that it's a map
                    throw std::runtime_error("Expected a map");
                }

                for (uint32_t i = 0; i < obj.via.map.size; ++i) {       // For all key-value in the map 
                    const msgpack::object_kv& kv = obj.via.map.ptr[i];  // Get the kv object

                    std::string key;
                    std::string value;

                    kv.key.convert(key);        // Extract the key    
                    kv.val.convert(value);      // Extract the value

                    msg.emplace(std::move(key), std::move(value));  // Insert directly with emplace
                }
                return;
            }
        } 
        else if (bytesReceived == 0) {
            std::cerr << "Connection closed by peer." << std::endl;
            return;
        } 
        else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "Receive timeout!" << std::endl;
                return;
           } else {
                perror("Receive failed");
                return;
            }
        }
    }
}

/// @brief Close the connection
void SocketModule::closeConnection() {
    if (connection_fd != -1){
        close(this->getConnectionFd()); 
        connection_fd = -1;
    }
    if (socket_fd != -1){ 
        close(this->getSocketFd()); 
        socket_fd = -1;
    }
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
