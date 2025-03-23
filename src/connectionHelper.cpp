#include "connectionHelper.hpp"

// Constructor: Initializes socket
SocketModule::SocketModule() : socket_fd(-1), connection_fd(-1) {}

// Initiates a client connection
bool SocketModule::initiateConnection(const std::string& ip, int port) {
    
    // Create a socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Socket creation failed");
        return false;
    }

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

// Waits for a client to connect (acts as a server)
bool SocketModule::waitForConnection(int port) {
    
    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Socket creation failed");
        return false;
    }

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
    return true;
}

// Send a message over the socket
void SocketModule::sendMessage(const json& message) {
    std::string jsonString = message.dump(); // Convert JSON to string
    send(this->connection_fd, jsonString.c_str(), jsonString.size(), 0);
}

// Read a message from the socket
json SocketModule::receiveMessage() {
    char buffer[1024] = {0};
    int bytesReceived = read(this->connection_fd, buffer, 1024);
    std::cout << "Bytes received: " << bytesReceived << std::endl;
    
    if (bytesReceived > 0) {
        std::string jsonString(buffer, bytesReceived);
        return json::parse(jsonString); // Parse and return JSON object
    }

    return json(); // Return empty JSON object if no data
}

// Close the connection
void SocketModule::closeConnection() {
    if (connection_fd != -1) close(connection_fd);
    if (socket_fd != -1) close(socket_fd);
}

// Destructor ensures the connection is closed
SocketModule::~SocketModule() {
    closeConnection();
}
