/**
 * @file 3_auth_client_key.cpp
 * @brief This file's goal is to measure the overheads of the authentication + key function. The output is the total time taken by the function to execute. 
 * If the project is compiled with -DMEASUREMENTS, The output also includes the active and idle time of the function execution.
 * 
 */

#include <string>
#include <chrono> 
#include <thread> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"
#include "../CycleCounter.hpp"

std::string idA = "A";
std::string idB = "B";

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
        std::cerr << "Error: No IP address provided. Please provide the IP as an argument." << std::endl;
        return 1;  // Exit with an error code
    }

    const char* ip = argv[1];  // Read IP from command-line argument

    std::string SA = std::string("6dec1165d79b505e442e72edfc64ecbe5f079fc97ad7ab0357db4bcb4d6dce57");
    unsigned char saltA[PUF_SIZE];
    fromHexString(SA, saltA, PUF_SIZE);

    std::string xBstr = std::string("988a5de27fcfd9607ff59ba4991b031ce3f77439d6896f13173c0dc7a81e9106");
    unsigned char xB[PUF_SIZE];
    fromHexString(xBstr, xB, PUF_SIZE);

    std::string RBstr = std::string("c399bc93cd39b3f31aefef929b079d8649986aaca2733c96f65880584cb428a8");
    unsigned char RB[PUF_SIZE];
    fromHexString(RBstr, RB, PUF_SIZE);

    std::string CAstr = std::string("1d03ccabf9a9ef443f1add71ba4eab953ac31f59142037d81fe9b63b3def9251");
    unsigned char CA[PUF_SIZE];
    fromHexString(CAstr, CA, PUF_SIZE);

    // Creation of the UAV
    UAV A(idA,saltA);
    A.addUAV(idB,xB,CA,RB);

    // Initiate connection    
    A.socketModule.initiateConnection(ip, 8080);
    
    // When the programm reaches this point, the UAV are connected
    
    // Warming up LibTomCrypt 
    warmup();    
    
    start = counter.getCycles(); 

    int ret = A.autentication_key_client();
    
    end = counter.getCycles(); 
    totalTime = end - start;

    if (ret == 1){
        // std::cout << "There was a problem" << std::endl;
        return ret;
    }

    std::cout << "Authentication + key procedure elapsed CPU cycles : " << totalTime << " cycles" << std::endl;
    A.socketModule.closeConnection();
    return 0;
}
