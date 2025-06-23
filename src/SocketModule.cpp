/**
 * @file SocketModule.cpp
 * @brief SocketModule implementation
 * 
 * This file holds the SocketModules class implementation.
 * 
 */

#include "SocketModule.hpp"

/// @brief Constructor: Initializes socket
SocketModule::SocketModule() : socket_fd(-1), connection_fd(-1), recv_buffer{{0},0}, send_buffer{{0},0} {
    cmp_init(&ctx_recv, &recv_buffer, _reader, NULL, NULL);
    cmp_init(&ctx_send, &send_buffer, NULL, NULL, _writer);
}

bool SocketModule::_reader(cmp_ctx_t *ctx, void *data, size_t count){
    CmpBuffer *recv_buffer = static_cast<CmpBuffer *>(ctx->buf);        // Static function cannot access instance members

    if (recv_buffer->cursor + count > BUFFER_SIZE){
        return false;
    }
    memcpy(data, &recv_buffer->buffer[recv_buffer->cursor], count);
    recv_buffer->cursor += count;       // Update cursor
    return true;
}

size_t SocketModule::_writer(cmp_ctx_t *ctx, const void *data, size_t count){
    CmpBuffer *send_buffer = static_cast<CmpBuffer *>(ctx->buf);         // Static function cannot access instance members

    if (send_buffer->cursor + count > BUFFER_SIZE){
        return false;
    }
    memcpy(&send_buffer->buffer[send_buffer->cursor], data, count);
    send_buffer->cursor += count;       // Update cursor
    return true;
}

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
    recv_buffer.cursor = 0;
    send_buffer.cursor = 0;
    return true;
}

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
    recv_buffer.cursor = 0;
    send_buffer.cursor = 0;
    return true;
}

void SocketModule::createMap(uint32_t size){
    cmp_write_map(&ctx_send, size);
}

void SocketModule::addKeyValue(const char  * key, const std::string &value) {
    cmp_write_str(&ctx_send, key, strlen(key));
    cmp_write_str(&ctx_send, value.c_str(), value.size());
}

void SocketModule::addKeyValue(const char  * key, const unsigned char *value, size_t valueLen) {
    cmp_write_str(&ctx_send, key, strlen(key));
    cmp_write_bin(&ctx_send, value, valueLen);
}

void SocketModule::addKeyValue(const char *key, const unsigned char L[CHALLENGE_SIZE][PUF_SIZE]) {
    cmp_write_str(&ctx_send, key, strlen(key));
    cmp_write_bin(&ctx_send, reinterpret_cast<const unsigned char*>(L), CHALLENGE_SIZE * PUF_SIZE);
}

void SocketModule::sendMsg(){
    send(this->connection_fd, send_buffer.buffer, send_buffer.cursor, 0);
    printMsg(send_buffer, send_buffer.cursor);
    send_buffer.cursor = 0; // Clear the buffer by moving the cursor
}

/**
 * @brief Receive a message on the msgPack format and return it in the unordered_map msg.  
 * 
 * @param msg 
 */
void SocketModule::receiveMsg(std::unordered_map<std::string,std::string> &msg) {
    // char buffer[1024] = {0};
    size_t bytesReceived = -1;

    if (!leftover_data.empty()) {
        bytesReceived = leftover_data.length();
        std::memcpy(recv_buffer.buffer, leftover_data.data(), bytesReceived);
        leftover_data.clear(); 
    }
    else {
        bytesReceived = read(this->connection_fd, recv_buffer.buffer, BUFFER_SIZE);
    }

    if (bytesReceived > 0 && bytesReceived != -1) {
        msg.clear();

        PROD_ONLY({ std::cout << "Received " << bytesReceived << " bytes." << std::endl; });
        
        // printMsg(recv_buffer,bytesReceived);
        
        uint32_t map_size;
        if (!cmp_read_map(&ctx_recv, &map_size)) {
            // Handle error: failed to read map header
            std::cerr << "Failed to read map from data : " << ctx_recv.error << "\n";
        }

        PROD_ONLY({std::cout << "Received message : " << std::endl;});

        for (uint32_t i = 0; i < map_size; i++) {
            uint32_t key_len = 64;
            char key[key_len];

            if (!cmp_read_str(&ctx_recv, key, &key_len)) {
                std::cerr << "Failed to read key data : " << ctx_recv.error << "\n";
                break;
            }
        
            uint32_t val_len = CHALLENGE_SIZE*PUF_SIZE;
            char val[val_len];

            size_t saved_cursor = recv_buffer.cursor;
            cmp_object_t obj;
            if (!cmp_read_object(&ctx_recv, &obj)) {
                // handle error
            }
            recv_buffer.cursor = saved_cursor;

            switch(obj.type) {
                case CMP_TYPE_BIN8:
                    if (!cmp_read_bin(&ctx_recv, val, &val_len)) {
                        std::cerr << "Failed to read val data : " << ctx_recv.error << "\n";
                        return;
                    }
                    PROD_ONLY({std::cout << key << " : "; print_hex((const unsigned char *)val,val_len);});
                    break;
                case CMP_TYPE_FIXSTR:
                    if (!cmp_read_str(&ctx_recv, val, &val_len)) {
                        std::cerr << "Failed to read val data : " << ctx_recv.error << "\n";
                        return;      
                    }
                    PROD_ONLY({std::cout << key << " : " << val << std::endl;});
                    break;
            }
        
            msg.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(std::string(key, key_len)),
                std::forward_as_tuple(std::string(val, val_len))  
            );
        }
        if (bytesReceived > recv_buffer.cursor){    // We received more data than we used now, maybe a second message.
            size_t remaining = bytesReceived - recv_buffer.cursor;
            leftover_data.assign(
                reinterpret_cast<char *>(recv_buffer.buffer + recv_buffer.cursor),
                remaining
            );
        }
        recv_buffer.cursor = 0;     // Clear the buffer by moving the cursor
        return ;

    } else if (bytesReceived == 0) {
        std::cerr << "Connection closed by peer." << std::endl;
        return;
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cerr << "Receive timeout!" << std::endl;
        } else {
            perror("Receive failed");
        }
        return;
    }
}

void SocketModule::printMsg(CmpBuffer &buffer, size_t size) {
    std::cout << "Sending message :" << std::endl;
    // std::cout << "Raw buffer = "; print_hex(buffer.buffer,size); std::cout << std::endl;
    // Create a temporary copy of the buffer for safe reading
    CmpBuffer temp_buffer;
    memcpy(temp_buffer.buffer, buffer.buffer, size);
    temp_buffer.cursor = 0;

    // Create a new CMP context with the temporary buffer
    cmp_ctx_t temp_ctx;
    cmp_init(&temp_ctx, &temp_buffer.buffer, _reader, NULL, NULL);

    uint32_t map_size = 0;
    if (!cmp_read_map(&temp_ctx, &map_size)) {
        printf("Failed to read map size: %d\n", temp_ctx.error);
        return;
    }

    for (uint32_t i = 0; i < map_size; i++) {
        uint32_t key_len = 64;
        char key[key_len];

        if (!cmp_read_str(&temp_ctx, key, &key_len)) {
            printf("Failed to read key data: %d\n", temp_ctx.error);
            break;
        }

        uint32_t val_len = CHALLENGE_SIZE*PUF_SIZE;
        char val[val_len];

        size_t saved_cursor = temp_buffer.cursor;
        cmp_object_t obj;
        if (!cmp_read_object(&temp_ctx, &obj)) {
            std::cerr << "Failed to read object: error code " << temp_ctx.error << std::endl;
            return;
        }
        temp_buffer.cursor = saved_cursor;

        switch(obj.type) {
            case CMP_TYPE_BIN8:
                if (!cmp_read_bin(&temp_ctx, val, &val_len)) {
                    std::cerr << "Failed to read val data : " << +temp_ctx.error << "\n";
                    return;
                }
                std::cout << key << ": "; print_hex(reinterpret_cast<unsigned char*>(val),val_len);
                break;
            case CMP_TYPE_FIXSTR:
                if (!cmp_read_str(&temp_ctx, val, &val_len)) {
                    std::cerr << "Failed to read val data : " << +temp_ctx.error << "\n";
                    return;
                }
                std::cout << key << ": " << val << std::endl;
                break;
        }
    }
    // std::cout << "Cursor at the end of print = " << temp_buffer.cursor << std::endl;
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
// 82a26964a24253a464617461c440c8176747eaec39d33d74d900d7187a9f624e7b30d2207a062a3bd5a1db4e66a826f23ea659a153d9e9452a98e31820945a6bf81d0997257853ae9fd8a07c9608
// 82a26964a24253a464617461c440c8176747eaec39d33d74d900d7187a9f624e7b30d2207a062a3bd5a1db4e66a826f23ea659a153d9e9452a98e31820945a6b