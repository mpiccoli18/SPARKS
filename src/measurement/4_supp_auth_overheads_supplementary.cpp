/**
 * @file 4_supp_auth_overheads_supplementary.cpp
 * @brief This file's goal is to measure the overheads of the supplementary authentication key function. The output is the total time taken by the function to execute. 
 * If the project is compiled with -DMEASUREMENTS, The output also includes the active and idle time of the function execution.
 * 
 */
#include <chrono> 
#include <thread>

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"
#include "../CycleCounter.hpp"

std::string idA = "A";
std::string idC = "C";
std::string idBS = "BS";

long long totalTime;
long long start;
long long end;

CycleCounter counter;

void fromHexString(const std::string& hex, unsigned char* output, size_t maxLength) {
    size_t length = hex.length() / 2;
    if (length > maxLength) length = maxLength; // Prevent buffer overflow

    for (size_t i = 0; i < length; i++) {
        std::string byteString = hex.substr(i * 2, 2);
        output[i] = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No IP address provided. Please provide the IP as argument." << std::endl;
        return 1;  // Exit with an error code
    }

    const char* ip = argv[1];  // Read IP from command-line argument

    // Creation of the UAV

    std::string SC = std::string("c310d59037f5dd19b7158b96583e1b2f5f421ee6489972b791c5d04b520783ef");
    unsigned char saltC[PUF_SIZE];
    fromHexString(SC, saltC, PUF_SIZE);
    UAV C(idC, saltC);
    
    std::string CAstr = std::string("a47af74e3f14bb6a62b2268acf4cd538d82e57f53946d4dee11f69d3f2c0a3f5");
    unsigned char CA[PUF_SIZE];
    fromHexString(CAstr, CA, PUF_SIZE);
    
    std::string xLstr = std::string("74228aa128afcecae551af5beb1ba0983fcb244ca7631e4f70be22b8732864dd");
    unsigned char xL[PUF_SIZE];
    fromHexString(xLstr, xL, PUF_SIZE);
    // std::cout << "xL : "; print_hex(xL, PUF_SIZE); std::cout << std::endl;
    
    std::string secretStr = std::string("15a5796e0d20e8288d6faef881969ed81677d628e6d500d5ba8f67906daa1e29");
    unsigned char secret[PUF_SIZE];
    fromHexString(secretStr, secret, PUF_SIZE);
    // std::cout << "secret : "; print_hex(secret, PUF_SIZE); std::cout << std::endl;
    
    C.addUAV(idA, nullptr, CA, nullptr, xL, secret);
    
    warmup();
    // Connect to the BS to retrieve A's credentials
    C.socketModule.initiateConnection(ip, 8080);
    
    start = counter.getCycles();
    int ret = C.supplementaryAuthenticationSup();
    end = counter.getCycles();
    totalTime = end - start;
    if (ret != 0){
        return ret;
    }

    std::cout << "Supplementary authentication procedure elapsed CPU cycles : " << totalTime << " cycles" << std::endl;
    return 0;
}