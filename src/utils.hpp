/**
 * @file utils.hpp
 * @brief Helper functions
 * 
 * This file is the header for useful functions throughout this project.
 * 
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>  // For std::ostringstream for rasp pi 4
#include <openssl/evp.h>
#include <iostream>
#include <random>
#include <iomanip>
#include <sstream>  // For std::ostringstream
#include <cstring> 
#include <nlohmann/json.hpp> 
#include <fstream>

#include <cryptopp/cryptlib.h>
#include <cryptopp/hkdf.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

using namespace CryptoPP;

using json = nlohmann::json;

#define PUF_SIZE 32 // 256 bits = 32 bytes
#define CHALLENGE_SIZE 5

/**
 * @brief Generates a random 256 bits unsigned char.
 * 
 * @param buffer 
 * @param size 
 */
void generate_random_bytes(unsigned char* buffer, size_t size = 32);

/**
 * @brief Print an unsigned char to a human readable format.
 * 
 * @param buffer 
 * @param length 
 */
void print_hex(const unsigned char *buffer, size_t length);

/**
 * @brief Compute the XOR operation between two buffers containing the code of two 32 bytes numbers.
 * 
 * @param input1 
 * @param input2 
 * @param size 
 * @param output 
 */
void xor_buffers(const unsigned char* input1, const unsigned char* input2, size_t size, unsigned char* output);

/**
 * @brief Initiate a hashing context
 * 
 * @return * Function* 
 */
EVP_MD_CTX* initHash();

/**
 * @brief Basinc variadic template that allow to add different types of data to a hash.
 * 
 * @param ctx 
 * @param value 
 * @return * Basic 
 */
template <typename T>
inline void addToHash(EVP_MD_CTX* ctx, const T& value){
    EVP_DigestUpdate(ctx, &value, sizeof(T));  
}

/**
 * @brief Specialization of the variadic template for buffers containing 32 Bytes numbers
 * 
 * @param ctx 
 * @param data 
 * @param size 
 */
void addToHash(EVP_MD_CTX* ctx, const unsigned char* data, size_t size);

/**
 * @brief Specialization of the variadic template for std::string type
 * 
 * @param ctx 
 * @param str 
 */
void addToHash(EVP_MD_CTX* ctx, const std::string& str);

/**
 * @brief Calculate the hash value with every elements added to the context
 * 
 * @param ctx 
 * @param output 
 */
void calculateHash(EVP_MD_CTX* ctx, unsigned char * output);

/**
 * @brief Print the content of a JSON value.
 * 
 * @param msg 
 */
void printJSON(json msg);

/**
 * @brief Transform an unsigned char buffer to a std::string for easier transportation
 * 
 * @param data 
 * @param length 
 * @return std::string 
 */
std::string toHexString(const unsigned char* data, size_t length);

/**
 * @brief Retrieve a unsigned char number fron a std::string
 * 
 * @param hex 
 * @param output 
 * @param maxLength 
 */
void fromHexString(const std::string& hex, unsigned char* output, size_t maxLength);


/**
 * @brief Try to get the current CPU Frequency. Might be skewed, only to be used as a support option.
 * 
 * @return double 
 */
double getCpuFrequency();

// Function to derive a key using HKDF (SHA256)
void deriveKeyUsingHKDF(const unsigned char* NA, const unsigned char* NB, const unsigned char* S, size_t keyLength, unsigned char* derivedKey);
#endif
