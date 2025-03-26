/**
 * @file utils.cpp
 * @brief Helper functions implementation
 * 
 * This file is the header for useful functions throughout this project.
 * 
 */


#include "utils.hpp"

void generate_random_bytes(unsigned char* buffer, size_t size) {
    std::random_device rd;
    std::mt19937_64 gen(rd()); // 64-bit generator

    size_t i = 0;
    while (i + 8 <= size) {  // Process in 8-byte chunks
        uint64_t rand_val = gen();
        std::memcpy(buffer + i, &rand_val, 8);
        i += 8;
    }

    // Handle remaining bytes (if size is not a multiple of 8)
    if (i < size) {
        uint64_t rand_val = gen();
        std::memcpy(buffer + i, &rand_val, size - i);
    }
}

void print_hex(const unsigned char *buffer, size_t length) {
    for (size_t i = 0; i < length; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i];
    }
    std::cout << std::dec << "\n";
}

void xor_buffers(const unsigned char* input1, const unsigned char* input2, size_t size, unsigned char* output) {
    for (size_t i = 0; i < size; ++i) {
        output[i] = input1[i] ^ input2[i];
    }
}

// Function that initialize a hash
EVP_MD_CTX* initHash(){
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    return ctx;
}

// Basic variadic template
// template <typename T>
// void addToHash(EVP_MD_CTX* ctx, const T& value){
//     EVP_DigestUpdate(ctx, &value, sizeof(T));  
// }

// const unsigned char * overload
void addToHash(EVP_MD_CTX* ctx, const unsigned char* data, size_t size){
    EVP_DigestUpdate(ctx, data, size);
}

// std::string overload
void addToHash(EVP_MD_CTX* ctx, const std::string& str){
    EVP_DigestUpdate(ctx, str.data(), str.size());
}

// // Multiple argument variadic template
// template <typename First, typename... Rest>
// void addToHash(EVP_MD_CTX* ctx, const First& first, const Rest&... rest){
//     addToHash(ctx, first);  // Process the first argument
//     (addToHash(ctx, rest), ...);  // Process the remaining arguments (fold expression)
// }

// Function that actually calculates the hash
void calculateHash(EVP_MD_CTX* ctx, unsigned char * output){
    EVP_DigestFinal_ex(ctx, output, NULL); 
    EVP_MD_CTX_free(ctx); 
}

void printJSON(json msg){
    if (msg.empty()) {
        std::cerr << "Error: JSON object is empty!" << std::endl;
    } else {
        std::cout << msg.dump(4) << std::endl;
    }
}

std::string toHexString(const unsigned char* data, size_t length) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; i++) {
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    return oss.str();
}

// Converts hex string back to unsigned char array
void fromHexString(const std::string& hex, unsigned char* output, size_t maxLength) {
    size_t length = hex.length() / 2;
    if (length > maxLength) length = maxLength; // Prevent buffer overflow

    for (size_t i = 0; i < length; i++) {
        std::string byteString = hex.substr(i * 2, 2);
        output[i] = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
    }
}
