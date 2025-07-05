/**
 * @file 2_auth_overheads_client.cpp
 * @brief This file's goal is to measure the overheads of the authentication function. The output is the total time taken by the function to execute. 
 * If the project is compiled with -DMEASUREMENTS, The output also includes the active and idle time of the function execution.
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No IP address provided. Please provide the IP as an argument." << std::endl;
        return 1;  // Exit with an error code
    }

    const char* ip = argv[1];  // Read IP from command-line argument

    // Creation of the UAV
    UAV A(idA);
    A.addUAV(idB);

    // Initiate connection    
    A.socketModule.initiateConnection(ip, 8080);
    
    // When the programm reaches this point, the UAV are connected
    
    // Warming up LibTomCrypt 
    warmup();    
    
    int ret = A.enrolment_client();
    if (ret == 1){
        return ret;
    }
    
    // We start the measurements
    start = counter.getCycles(); 

    ret = A.autentication_client();
    
    end = counter.getCycles(); 
    totalTime = end - start;

    if (ret == 1){
        std::cout << "There was a problem" << std::endl;
        return ret;
    }

    std::cout << "Authentication procedure elapsed CPU cycles : " << totalTime << " cycles" << std::endl;
    A.socketModule.closeConnection();
    return 0;
}
