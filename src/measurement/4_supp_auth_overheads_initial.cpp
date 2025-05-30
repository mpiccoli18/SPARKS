/**
 * @file 4_supp_auth_initial.cpp
 * @brief This file's goal is to measure the overheads of the supplementary authentication key function. The output is the total time taken by the function to execute. 
 * If the project is compiled with -DMEASUREMENTS, The output also includes the active and idle time of the function execution.
 * 
 */

#include <string>
#include <chrono> 

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"
#include "../CycleCounter.hpp"

std::string idA = "A";
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

int main() {

    // Creation of the UAV

    std::string SA = std::string("dc5792cd19dc40bb7433b9309809d1f1e042832e075d431d5d6c622fc5ee7544");
    unsigned char saltA[PUF_SIZE];
    fromHexString(SA, saltA, PUF_SIZE);
    UAV A(idA, saltA);

    // Warming up LibTomCrypt 
    warmup();

    // Initiate connection
    A.socketModule.waitForConnection(8080);

    // When the programm reaches this point, the UAV are connected

    start = counter.getCycles();
    int ret = A.supplementaryAuthenticationInitial();
    end = counter.getCycles();
    totalTime = end - start;
    if (ret != 0){
        return ret;
    }

    std::cout << "Supplementary authentication procedure elapsed CPU cycles : " << totalTime << " cycles" << std::endl;
    return 0;
}