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
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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

    std::cout << "Waiting for a connection on port " << port << "...\n";

    socklen_t addr_len = sizeof(address);
    connection_fd = accept(socket_fd, (struct sockaddr*)&address, &addr_len);
    if (connection_fd < 0) {
        perror("Accept failed");
        return false;
    }

    std::cout << "Client connected!\n";

    // Set the timeout
    setsockopt(connection_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(connection_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    return true;
}

/// @brief Send a message over the socket
void SocketModule::sendMessage(const json& message) {
    std::string jsonString = message.dump(); // Convert JSON to string
    send(this->connection_fd, jsonString.c_str(), jsonString.size(), 0);
}

/// @brief Receive a message on the connection socket.
json SocketModule::receiveMessage() {
    static std::string dataBuffer = "";  // Store partial data between calls
    char buffer[1024] = {0};
    
    while (true) {
        int bytesReceived = read(this->connection_fd, buffer, sizeof(buffer));
        // std::cout << "Bytes received: " << bytesReceived << std::endl;

        if (bytesReceived > 0) {
            // Append new data to buffer
            dataBuffer += std::string(buffer, bytesReceived);

            try {
                // Attempt to parse JSON
                json parsedJson = json::parse(dataBuffer);
                dataBuffer.clear();  // Clear buffer after successful parsing
                return parsedJson;
            } catch (json::parse_error&) {
                std::cerr << "Partial JSON received, waiting for more data..." << std::endl;
                continue;  // Keep reading until we get a full JSON
            }
        } 
        else if (bytesReceived == 0) {
            std::cerr << "Connection closed by peer." << std::endl;
            return json({{"error", "connection_closed"}});
        } 
        else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "Receive timeout!" << std::endl;
                return json({{"error", "timeout"}});
            } else {
                perror("Receive failed");
                return json({{"error", "read_failed"}});
            }
        }
    }
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
