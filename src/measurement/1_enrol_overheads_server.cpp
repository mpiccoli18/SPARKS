/**
 * @file 1_enrol_server.cpp
 * @brief This file's goal is to measure the overheads of the enrolment function. The output is the total time taken by the function to execute. 
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

int main() {
    // Creation of the UAV

    UAV A(idA);
    A.addUAV(idB);

    // Listen for a connection
    A.socketModule.waitForConnection(8080);
    
    // When the programm reaches this point, the UAV are connected
    
    // Warming up LibTomCrypt
    warmup();    
    
    // We start the measurements
    start = counter.getCycles();
    int ret = A.enrolment_server();
    if (ret == 1){
        return ret;
    }
    end = counter.getCycles();
    totalTime = end - start;

    std::cout << "Enrolment procedure elapsed CPU cycles : " << totalTime << " cycles" << std::endl;
    A.socketModule.closeConnection();
    return 0;
}
