/**
 * @file SocketModule.hpp
 * @brief SocketModule header
 * 
 * This file holds the SocketModules class header.
 * 
 */

#ifndef SocketModule_HPP
#define SocketModule_HPP

#include <arpa/inet.h>

#include "cmp/cmp.h"
#include "utils.hpp"

#define TIMEOUT_VALUE   5
#define BUFFER_SIZE     512

struct CmpBuffer {
    uint8_t buffer[BUFFER_SIZE];
    size_t cursor;
};

/// @brief Socket module class. Its job is to manage everything connection related for a server and a client.
class SocketModule {
private:
    int socket_fd;          // Socket file descriptor
    int connection_fd;      // Used when acting as a server
    struct sockaddr_in address;
    cmp_ctx_t ctx_recv;     // cmp receive context
    cmp_ctx_t ctx_send;     // cmp sending context
    CmpBuffer recv_buffer;  // cmp receive buffer structure containing the buffer and a cursor
    CmpBuffer send_buffer;  // cmp sending buffer structure containing the buffer and a cursor
    std::string leftover_data; // Used to store supplementary messages recieved too early

    static bool _reader(cmp_ctx_t *ctx, void *data, size_t count);          // cmp internal reading function
    static size_t _writer(cmp_ctx_t *ctx, const void *data, size_t count);  // cmp internal writting function

public:
    SocketModule();  // Constructor
    
    // Delete copy constructor and copy assignment operator
    SocketModule(const SocketModule&) = delete;
    SocketModule& operator=(const SocketModule&) = delete;

    // Delete move constructor and move assignment operator
    SocketModule(SocketModule&&) = delete;
    SocketModule& operator=(SocketModule&&) = delete;


    ~SocketModule(); // Destructor

    /**
     * @brief initiate a connection at the address "ip" on the port "port"
     * 
     * @param ip 
     * @param port 
     * @return true 
     * @return false 
     */
    bool initiateConnection(const std::string& ip, int port);

    /**
     * @brief Wait for a connection on the port "port"
     * 
     * @param port 
     * @return true 
     * @return false 
     */
    bool waitForConnection(int port);
    
    /**
     * @brief Create a map of size "size" into the sending buffer
     * 
     * @param size 
     */
    void createMap(uint32_t size);

    /**
     * @brief Add a key value to the sending buffer. This overload adds a std::string to the buffer
     * 
     * @param key 
     * @param value 
     */
    void addKeyValue(const char * key, const std::string &value);

    /**
     * @brief Add a key value to the sending buffer. This overload adds an unsigned char * to the buffer
     * 
     * @param key 
     * @param value 
     * @param valueLen 
     */
    void addKeyValue(const char * key, const unsigned char *value, size_t valueLen);

    /**
     * @brief  Add a key value to the sending buffer. This overload adds an array of unsigned char * to the buffer
     * 
     * @param key 
     * @param LC 
     */
    void addKeyValue(const char * key, const unsigned char LC[CHALLENGE_SIZE][PUF_SIZE]);

    /**
     * @brief Send a msgPack over the socket.
     * 
     * @param ctx cmp context of the message to send
     * @param len Length of the message to send
     */
    void sendMsg();

    /**
     * @brief Receive a message on the msgPack format and return it in the unordered_map "msg".  
     * 
     * @param msg 
     */
    void receiveMsg(std::unordered_map<std::string,std::string> &msg);

    /**
     * @brief Print the content of a MsgPack value.
     * 
     * @param msg 
     */
    void printMsg(CmpBuffer &buffer, size_t size);

    /**
     * @brief Close the connection
     * 
     */
    void closeConnection();

    /**
     * @brief Check is a connection is established
     * 
     * @return true 
     * @return false 
     */
    bool isOpen() const;

    /**
     * @brief Returns the socket Fd object
     * 
     * @return int 
     */
    int getSocketFd() const;

    /**
     * @brief Returns the connection Fd object
     * 
     * @return int 
     */
    int getConnectionFd() const;
};

#endif
