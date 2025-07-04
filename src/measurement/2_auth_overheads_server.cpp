/**
 * @file 2_auth_server.cpp
 * @brief This file's goal is to measure the overheads of the authentication function. The output is the total time taken by the function to execute. 
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

int main(){

    // Creation of the UAV
    
    std::string SB = std::string("bbccfa7fc3a347792a7f5564b3429002e31069840d2bba1b61beb1d700e02b57");
    unsigned char saltB[PUF_SIZE];
    fromHexString(SB, saltB, PUF_SIZE);

    std::string xAstr = std::string("8fdaf6ff0f4f73a9d64f1b94967d051d75bef17ce457e7bf6f36c5f0332bc691");
    unsigned char xA[PUF_SIZE];
    fromHexString(xAstr, xA, PUF_SIZE);

    std::string RAstr = std::string("28d28185962fae1c07d9a8fccefb53d03e5797c1539f4df253f5ef1b84295cc5");
    unsigned char RA[PUF_SIZE];
    fromHexString(RAstr, RA, PUF_SIZE);

    std::string CBstr = std::string("1bfbb8c02c67819aa8ce2c5425d3c7f7b62d1d683e1c8c126e7b41bd6f0e3b4d");
    unsigned char CB[PUF_SIZE];
    fromHexString(CBstr, CB, PUF_SIZE);

    // Creation of the UAV
    UAV B(idB,saltB);
    B.addUAV(idA,xA,CB,RA);

    B.socketModule.waitForConnection(8080);

    // When the programm reaches this point, the UAV are connected

    // Warming up LibTomCrypt 
    warmup();

    start = counter.getCycles(); 
    
    int ret = B.autentication_server();
    
    end = counter.getCycles(); 
    totalTime= end - start;

    if (ret == 1){
        return ret;
    }

    std::cout << "Authentication procedure elapsed CPU cycles : " << totalTime << " cycles" << std::endl;
    B.socketModule.closeConnection();

    return 0;
}
