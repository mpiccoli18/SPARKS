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
bool server = false;

long long totalTime;
long long start;
long long end;


CycleCounter counter;

void warmup(UAV * A){
    unsigned char rand[PUF_SIZE];
    generate_random_bytes(rand);
    unsigned char out[PUF_SIZE];
    A->callPUF(rand, out);
    // print_hex(out, PUF_SIZE);
    register_hash(&sha256_desc);
}

int main() {
    // Creation of the UAV

    UAV A = UAV(idA);
    A.addUAV(idB);

    // std::cout << "The client drone id is : " <<A.getId() << ".\n"; 
    
    A.socketModule.waitForConnection(8080);
    
    // When the programm reaches this point, the UAV are connected
    
    // We now warm up the functions 
    warmup(&A);    
    
    // We start the measurements
    start = counter.getCycles();
    int ret = A.enrolment_server();
    if (ret == 1){
        return ret;
    }
    end = counter.getCycles();
    totalTime = end - start;

    A.socketModule.closeConnection();

    std::cout << "Enrolment procedure elapsed CPU cycles : " << totalTime << " cycles" << std::endl;
    // std::cout << "operational Elapsed CPU cycles passive enrolment: " << opCyclesPassive << " cycles" << std::endl;
    // std::cout << "idle Elapsed CPU cycles passive enrolment: " << idlcyclesPassive << " cycles" << std::endl;

    // std::cout << "Elapsed CPU cycles active enrolment: " << totalActive << " cycles" << std::endl;
    // std::cout << "operational Elapsed CPU cycles active enrolment: " << opCyclesActive << " cycles" << std::endl;
    // std::cout << "idle Elapsed CPU cycles active enrolment: " << idlcyclesActive << " cycles" << std::endl;
    A.socketModule.closeConnection();

    return 0;
}


// auto start = counter.getCycles(); 
// auto end = counter.getCycles();
// auto duration = end - start;
// std::cout << "Execution time for enrolment : " << duration.count() << " microseconds" << std::endl;


// start = counter.getCycles(); 
// end = counter.getCycles();
// duration = end - start;
// std::cout << "Execution time for authentication : " << duration.count() << " microseconds" << std::endl;

// uint64_t start = rdtsc();
        
// uint64_t end = rdtsc();

// std::cout << "Execution time (in CPU cycles): " << (end - start) << " cycles\n";
