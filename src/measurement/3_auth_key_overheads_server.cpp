/**
 * @file 3_auth_server_key.cpp
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

int main(){

    // Creation of the UAV

    UAV B(idB);

    B.socketModule.waitForConnection(8080);

    // When the programm reaches this point, the UAV are connected

    // Warming up LibTomCrypt 
    warmup();

    int ret = B.enrolment_server();
    if (ret == 1){
        return ret;
    }

    
    start = counter.getCycles(); 
    
    ret = B.autentication_key_server();
    
    end = counter.getCycles(); 
    totalTime = end - start;

    if (ret == 1){
        return ret;
    }

    std::cout << "Authentication + key procedure elapsed CPU cycles : " << totalTime << " cycles" << std::endl;
    B.socketModule.closeConnection();
    return 0;
}
