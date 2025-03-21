#ifndef UTILS_HPP
#define UTILS_HPP

#include <openssl/evp.h>
#include <iostream>
#include <random>
#include <iomanip>
#include <cstring> 

#define PUF_SIZE 32 // 256 bits = 32 bytes

void generate_random_bytes(unsigned char* buffer, size_t size = 32);

void print_hex(const unsigned char *buffer, size_t length);

void xor_buffers(const unsigned char* input1, const unsigned char* input2, size_t size, unsigned char* output);

// Function that initialize a hash
EVP_MD_CTX* initHash();

// Basic variadic template
template <typename T>
inline void addToHash(EVP_MD_CTX* ctx, const T& value){
    EVP_DigestUpdate(ctx, &value, sizeof(T));  
}

// const unsigned char * overload
void addToHash(EVP_MD_CTX* ctx, const unsigned char* data, size_t size);

// std::string overload
void addToHash(EVP_MD_CTX* ctx, const std::string& str);

// // Multiple argument variadic template
// template <typename First, typename... Rest>
// void addToHash(EVP_MD_CTX* ctx, const First& first, const Rest&... rest);

// Function that actually calculates the hash
void calculateHash(EVP_MD_CTX* ctx, unsigned char * output);

#endif