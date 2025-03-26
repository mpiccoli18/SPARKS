/**
 * @file puf.cpp
 * @brief Puf class implementation.
 * 
 * This file holds the puf class implementation.
 * 
 */

#include "puf.hpp"

/// @brief This function represent the coputation of the PUF. SHA256 is a one-way function used to simulate a PUF behaviour.
void sha256_raw(const unsigned char* data, size_t len, const unsigned char * salt, size_t saltSize, unsigned char* output) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();  // Create a new context
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);  // Initialize with SHA-256
    if(salt != NULL){
        EVP_DigestUpdate(ctx, salt, saltSize);
    }
    EVP_DigestUpdate(ctx, data, len);  // Update with data
    EVP_DigestFinal_ex(ctx, output, NULL);  // Finalize and get hash
    EVP_MD_CTX_free(ctx);  // Free the context
}

/// @brief Constructor
puf::puf() : salt{} { 
    generate_random_bytes(const_cast<unsigned char*>(salt), PUF_SIZE);
}

/// @brief Initiate the PUF computation.
/// @param input 
/// @param size 
/// @param output 
void puf::process(const unsigned char * input, size_t size, unsigned char * output) const{
    sha256_raw(input, size, this->salt, sizeof(this->salt), output);
}




