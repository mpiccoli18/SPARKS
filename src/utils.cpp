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

// Function to read the CPU frequency from /proc/cpuinfo
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
