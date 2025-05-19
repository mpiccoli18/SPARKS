/**
 * @file SocketModule.hpp
 * @brief SocketModule header
 * 
 * This file holds the SocketModules class header.
 * 
 */

#ifndef SocketModule_HPP
#define SocketModule_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#if defined(__linux__)
#include <arpa/inet.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <Winsock2.h>
#include <ws2tcpip.h>
#endif
#include <nlohmann/json>

using json = nlohmann::json;
#define TIMEOUT_VALUE  5

/// @brief Socket module class. Its job is to manage everything connection related for a server and a client.
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
