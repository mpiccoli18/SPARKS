/**
 * @file utils.cpp
 * @brief Helper functions implementation
 * 
 * This file is the header for useful functions throughout this project.
 * 
 */


#include "utils.hpp"

/**
 * @brief Generates a random 256 bits unsigned char.
 * 
 * @param buffer 
 * @param size 
 */
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

/**
 * @brief Print an unsigned char to a human readable format.
 * 
 * @param buffer 
 * @param length 
 */
void print_hex(const unsigned char *buffer, size_t length) {
    for (size_t i = 0; i < length; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i];
    }
    std::cout << std::dec << "\n";
}

/**
 * @brief Compute the XOR operation between two buffers containing the code of two 32 bytes numbers.
 * 
 * @param input1 
 * @param input2 
 * @param size 
 * @param output 
 */
void xor_buffers(const unsigned char* input1, const unsigned char* input2, size_t size, unsigned char* output) {
    if (output == input1 || output == input2) {
        // XOR into a temporary buffer to avoid interference
        unsigned char* temp_output = new unsigned char[size];
        
        for (size_t i = 0; i < size; ++i) {
            temp_output[i] = input1[i] ^ input2[i];
        }
        
        // Copy the result back to the original output buffer if needed
        if (output == input1) {
            for (size_t i = 0; i < size; ++i) {
                output[i] = temp_output[i];
            }
        } else {
            for (size_t i = 0; i < size; ++i) {
                output[i] = temp_output[i];
            }
        }

        delete[] temp_output;  // Clean up
    } else {
        // No interference, XOR directly into the output buffer
        for (size_t i = 0; i < size; ++i) {
            output[i] = input1[i] ^ input2[i];
        }
    }
}

/**
 * @brief Initiate a hashing context
 * 
 * @return * Function* 
 */hash_state* initHash(){
    hash_state* ctx = new hash_state();
    sha256_init(ctx);
    return ctx;
}

/**
 * @brief Specialization of the variadic template for buffers containing 32 Bytes numbers
 * 
 * @param ctx 
 * @param data 
 * @param size 
 */
void addToHash(hash_state* ctx, const unsigned char* data, size_t size){
    sha256_process(ctx, data, size);
}

/**
 * @brief Specialization of the variadic template for std::string type
 * 
 * @param ctx 
 * @param str 
 */void addToHash(hash_state* ctx, const std::string& str){
    sha256_process(ctx, reinterpret_cast<const unsigned char*>(str.data()), str.size());
}

/**
 * @brief Calculate the hash value with every elements added to the context
 * 
 * @param ctx 
 * @param output 
 */
void calculateHash(hash_state* ctx, unsigned char * output){
    sha256_done(ctx, output);
    delete ctx;
}

/**
 * @brief Print the content of a MsgPack data.
 * 
 * @param msg 
 */
void printMsgPack(std::unordered_map<std::string, std::string> data){
    if(data.empty()){
        std::cerr << "Error: MsgPack data is empty!" << std::endl;
        return;
    }
    
    std::cout << "MsgPack content:\n";
    for (const auto& pair : data) {
        const std::string& key = pair.first;
        const std::string& value = pair.second;

        std::cout << "  " << key << ": ";

        if (key == "id") {
            // Always treat "id" as printable text
            std::cout << value;
        } else {
            // Print other values as hex
            std::cout << "[" << value.size() << " bytes] ";
            for (unsigned char c : value) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)c;
            }
            std::cout << std::dec;  // reset to decimal
        }

        std::cout << "\n";
    }
}

/**
 * @brief Try to get the current CPU Frequency. Might be skewed, only to be used as a support option.
 * 
 * @return double 
 */
double getCpuFrequency() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    double frequency = 0.0;

    while (std::getline(cpuinfo, line)) {
        if (line.find("cpu MHz") != std::string::npos) {
            // Extract the frequency (in MHz) from the line
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                frequency = std::stod(line.substr(pos + 1));
                break;
            }
        }
    }

    if (frequency == 0.0) {
        // If no frequency was found, try reading from cpufreq
        std::ifstream freqFile("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq");
        if (freqFile.is_open()) {
            freqFile >> frequency;
            frequency /= 1000;
            freqFile.close();
        }
    }

    // If still no frequency, return a default value (fallback)
    if (frequency == 0.0) {
        frequency = 1.0;  // Default to 1 GHz if nothing is found
    }

    return frequency / 1000.0;  // Convert MHz to GHz
}

/*
// Function to derive a key using HKDF (SHA256)
void deriveKeyUsingHKDF(const unsigned char* NA, const unsigned char* NB, const unsigned char* S,
    size_t keyLength, unsigned char* derivedKey) {
    // Combine NA and NB into the input key material (IKM)
    unsigned char input_key_material[64];  // 32 bytes + 32 bytes = 64 bytes
    std::memcpy(input_key_material, NA, 32);
    std::memcpy(input_key_material + 32, NB, 32);

    // Prepare for HKDF with SHA256 as the hash function
    HKDF<SHA256> hkdf;

    // Define the salt and info for HKDF
    SecByteBlock salt(S, 32);  // 32 bytes salt
    SecByteBlock info;         // Optional info, can be empty (no extra context data)

    // Derive the key
    hkdf.DeriveKey(derivedKey, keyLength, input_key_material, sizeof(input_key_material), salt, salt.size(), info, info.size());
}
*/

/// @brief Function to derive a key using HKDF with the help of LibTomCrypt(SHA256)
/// @param NA Nonce A
/// @param NB Nonce B
/// @param S Salt
/// @param keyLength Length of the derived key
/// @param derivedKey Buffer to store the derived key
void deriveKeyUsingHKDF(const unsigned char* NA, const unsigned char* NB, const unsigned char* S,
    size_t keyLength, unsigned char* derivedKey) {
    // Combine NA and NB into the input key material (IKM)
    int err, sha256_idx;
    unsigned long keyLen = keyLength;
    unsigned char input_key_material[64];  // 32 bytes + 32 bytes = 64 bytes
    std::memcpy(input_key_material, NA, 32);
    std::memcpy(input_key_material + 32, NB, 32);

    // Prepare for HKDF with SHA256 as the hash function
    if(register_hash(&sha256_desc) != CRYPT_OK) {
        std::cerr << "Error registering SHA256 hash function." << std::endl;
        return;
    }

    sha256_idx = find_hash("sha256");
    /*if((err = hmac_memory(sha256_idx, input_key_material, sizeof(input_key_material), S, sizeof(S), derivedKey, &keyLen)) != CRYPT_OK) {
        std::cerr << "Error deriving key using HKDF: " << error_to_string(err) << std::endl;
        return;
    }*/

    if((err = hkdf(sha256_idx, S, 32, nullptr, 0, input_key_material, sizeof(input_key_material), derivedKey, keyLen)) != CRYPT_OK) {
        std::cerr << "Error deriving key using HKDF: " << error_to_string(err) << std::endl;
        return;
    }
}

bool extractValueFromMap(std::unordered_map<std::string, std::string> map, std::string key , unsigned char * output, size_t size){

    auto it = map.find(key);
    if (it == map.end()) {
        std::cerr << "Error: key " << key << " not found.\n";
        return false;
    }

    const std::string& valStr = it->second;

    if (valStr.size() != size) {
        std::cerr << "Error: value has incorrect size (" << valStr.size() << ").\n";
        return false;
    }
    std::memcpy(output, valStr.data(), size);
    return true;
}

void warmup(){
    register_hash(&sha256_desc);
}