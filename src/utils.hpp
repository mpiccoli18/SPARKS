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
#include <iostream>
#include <random>
#include <iomanip>
#include <sstream>  // For std::ostringstream
#include <cstring> 
#include <msgpack.hpp>
#include <fstream>
#include <tomcrypt.h>

#define PUF_SIZE 32 // 256 bits = 32 bytes
#define CHALLENGE_SIZE 5

//// MACRO

#pragma once

#if defined(MEASUREMENTS)

    // Basic measurements: exclude both measurement and prod-only logs
    #define MEASURE_ONLY(code) do {} while (0)
    #define PROD_ONLY(code)    do {} while (0)

#elif defined(MEASUREMENTS_DETAILLED)

    // Detailed measurements: include measurement code, exclude prod-only logs
    #define MEASURE_ONLY(code) do { code } while (0)
    #define PROD_ONLY(code)    do {} while (0)

#else

    // Production: exclude measurement code, include prod-only logs
    #define MEASURE_ONLY(code) do {} while (0)
    #define PROD_ONLY(code)    do { code } while (0)

#endif


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
hash_state* initHash();

/**
 * @brief Basinc variadic template that allow to add different types of data to a hash.
 * 
 * @param ctx 
 * @param value 
 * @return * Basic 
 */
template <typename T>
inline void addToHash(hash_state* ctx, const T& value){
    sha256_process(ctx, reinterpret_cast<const unsigned char*>(&value), sizeof(T));
}

/**
 * @brief Specialization of the variadic template for buffers containing 32 Bytes numbers
 * 
 * @param ctx 
 * @param data 
 * @param size 
 */
void addToHash(hash_state* ctx, const unsigned char* data, size_t size);

/**
 * @brief Specialization of the variadic template for std::string type
 * 
 * @param ctx 
 * @param str 
 */
void addToHash(hash_state* ctx, const std::string& str);

/**
 * @brief Calculate the hash value with every elements added to the context
 * 
 * @param ctx 
 * @param output 
 */
void calculateHash(hash_state* ctx, unsigned char * output);

/**
 * @brief Print the content of a MsgPack value.
 * 
 * @param msg 
 */
void printMsg(std::unordered_map<std::string, std::string> data);

/**
 * @brief Try to get the current CPU Frequency. Might be skewed, only to be used as a support option.
 * 
 * @return double 
 */
double getCpuFrequency();

/**
 * @brief Function to derive a key using HKDF (SHA256)
 * 
 * @param NA 
 * @param NB 
 * @param S 
 * @param keyLength 
 * @param derivedKey 
 * @return * void 
 */
void deriveKeyUsingHKDF(const unsigned char* NA, const unsigned char* NB, const unsigned char* S, size_t keyLength, unsigned char* derivedKey);
#endif

/**
 * @brief This function is a helper to extract a unsigned char table 'output' indexed at 'key' an unordered map 'map'. 
 * 
 * @param map 
 * @param key 
 * @param output 
 * @param size 
 * @return true 
 * @return false 
 */
bool extractValueFromMap(std::unordered_map<std::string, std::string> map, std::string key , unsigned char * output, size_t size);

/**
 * @brief Warmup for LibTomCrypt
 * 
 */
void warmup();
