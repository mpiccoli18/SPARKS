#ifndef CONNECTIONHELPER_HPP
#define CONNECTIONHELPER_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <nlohmann/json.hpp> 

using json = nlohmann::json;

class SocketModule {
private:
    int socket_fd;         // Socket file descriptor
    int connection_fd;     // Used when acting as a server
    struct sockaddr_in address;

public:
    SocketModule();  // Constructor
    ~SocketModule(); // Destructor

    bool initiateConnection(const std::string& ip, int port);
    bool waitForConnection(int port);
    
    void sendMessage(const json& message);
    json receiveMessage(); 

    void closeConnection();
};

#endif
