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
#include <arpa/inet.h>
#include <msgpack.hpp>

#include "utils.hpp"

#define TIMEOUT_VALUE  5

/// @brief Socket module class. Its job is to manage everything connection related for a server and a client.
class SocketModule {
private:
    int socket_fd;         // Socket file descriptor
    int connection_fd;     // Used when acting as a server
    struct sockaddr_in address;
    std::string leftover_data; // Used to store supplementary messages recieved too early
public:
    SocketModule();  // Constructor
    
    // Delete copy constructor and copy assignment operator
    SocketModule(const SocketModule&) = delete;
    SocketModule& operator=(const SocketModule&) = delete;

    // Delete move constructor and move assignment operator
    SocketModule(SocketModule&&) = delete;
    SocketModule& operator=(SocketModule&&) = delete;


    ~SocketModule(); // Destructor

    bool initiateConnection(const std::string& ip, int port);
    bool waitForConnection(int port);
    
    void sendMsg(const std::unordered_map<std::string, std::string> &msg);
    void receiveMsg(std::unordered_map<std::string, std::string> &msg);

    void closeConnection();
    bool isOpen() const;
    int getSocketFd() const;
    int getConnectionFd() const;
};

#endif
