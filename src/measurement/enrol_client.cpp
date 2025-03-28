#include <string>
#include <chrono> 
#include <x86intrin.h>
#include <thread>

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idA = "A";
std::string idB = "B";
bool server = false;
unsigned int pid;
uint64_t start;
uint64_t end;
uint64_t m1;
uint64_t m2;
uint64_t m3;
uint64_t m4;
uint64_t m5;
uint64_t m6;
uint64_t m7;


void warmup(UAV * A){
    unsigned char rand[PUF_SIZE];
    generate_random_bytes(rand);
    unsigned char out[PUF_SIZE];
    A->callPUF(rand, out);
    print_hex(out, PUF_SIZE);
}

int enrolment_client_1(UAV * A){
    
    // std::cout << "\nEnrolment process begins.\n";
    
    // A enroll with B
    // A computes the challenge for B
    start = __rdtscp(&pid);
    unsigned char xB[PUF_SIZE];
    generate_random_bytes(xB, PUF_SIZE);
    // std::cout << "xB : "; print_hex(xB, PUF_SIZE);
    
    // Creates B in the memory of A and save xB 
    A->getUAVData(idB)->setX(xB);
    
    // Creates the challenge for B
    unsigned char CB[PUF_SIZE];
    A->callPUF(xB, CB);
    // std::cout << "CB : "; print_hex(CB, PUF_SIZE);
    
    // Sends CB
    json msg = {{"id", idA}, {"C", toHexString(CB, PUF_SIZE)}};
    end = __rdtscp(&pid);
    m1 = end - start;
    start = __rdtscp(&pid);
    A->socketModule.sendMessage(msg);
    // std::cout << "Sent CB.\n";
    
    // Wait for B's response (with RB)
    json rsp = A->socketModule.receiveMessage();
    // printJSON(rsp);
    
    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }
    
    unsigned char RB[PUF_SIZE];
    if(!rsp.contains("R")){
        std::cerr << "Error occurred: no member R" << std::endl;
        return 1;
    }
    fromHexString(rsp["R"].get<std::string>(), RB, PUF_SIZE);
    end = __rdtscp(&pid);
    m2 = end - start;
    start = __rdtscp(&pid);
    A->getUAVData(idB)->setR(RB);
    end = __rdtscp(&pid);
    m3 = end - start;
    // std::cout << "\nB is enroled to A\n";
   
    return 0;
}

int enrolment_client_2(UAV * A){
    start = __rdtsc();
    // B enroll with A
    // A receive CA. It saves CA.
    json rsp = A->socketModule.receiveMessage();
    // printJSON(rsp);

    // Check if an error occurred
    if (rsp.contains("error")) {
        std::cerr << "Error occurred: " << rsp["error"] << std::endl;
        return -1;
    }

    unsigned char CA[PUF_SIZE];
    if(!rsp.contains("C")){
        std::cerr << "Error occurred: no member C" << std::endl;
        return 1;
    }
    fromHexString(rsp["C"].get<std::string>(), CA, PUF_SIZE);
    end = __rdtsc();
    m4 = end - start;

    start = __rdtsc();
    A->getUAVData(idB)->setC(CA);

    // A computes RA
    unsigned char RA[PUF_SIZE];
    A->callPUF(CA, RA);
    // std::cout << "RA : "; print_hex(RA, PUF_SIZE);

    end = __rdtsc();
    m5 = end - start;
    // A sends RA
    json msg = {{"id", idA}, {"R", toHexString(RA, PUF_SIZE)}};
    A->socketModule.sendMessage(msg);
    // std::cout << "Sent RA.\n";

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No IP address provided. Please provide the IP as an argument." << std::endl;
        return 1;  // Exit with an error code
    }

    const char* ip = argv[1];  // Read IP from command-line argument

    std::cout << "Using IP: " << ip << std::endl;

    // Creation of the UAV

    UAV A = UAV(idA);
    A.addUAV(idB);

    std::cout << "The client drone id is : " <<A.getId() << ".\n"; 
    
    A.socketModule.initiateConnection(ip, 8080);
    
    // When the programm reaches this point, the UAV are connected
    
    // We now warm up the functions 
    warmup(&A);    
    
    std::cout << "Started enrolment one side" << std::endl;
    start = __rdtsc();
    int ret = enrolment_client_1(&A);
    if (ret == 1){
        return ret;
    }
    end = __rdtsc();
    m6 = end - start;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    start = __rdtsc();
    ret = enrolment_client_2(&A);
    if (ret == 1){
        return ret;
    }
    end = __rdtsc();
    m7 = end - start;
    
    std::cout << "Finished enrolment" << std::endl;

    std::cout << "m1 Elapsed CPU cycles: " << m1 << " cycles" << std::endl;
    std::cout << "m2 Elapsed CPU cycles: " << m2 << " cycles" << std::endl;
    std::cout << "m3 Elapsed CPU cycles: " << m3 << " cycles" << std::endl;
    std::cout << "m4 Elapsed CPU cycles: " << m4 << " cycles" << std::endl;
    std::cout << "m5 Elapsed CPU cycles: " << m5 << " cycles" << std::endl;
    std::cout << "m6 Elapsed CPU cycles: " << m6 << " cycles" << std::endl;
    std::cout << "m7 Elapsed CPU cycles: " << m6 << " cycles" << std::endl;
    A.socketModule.closeConnection();

    return 0;
}

// auto start = std::chrono::high_resolution_clock::now(); 
// auto end = std::chrono::high_resolution_clock::now();
// auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
// std::cout << "Execution time for enrolment : " << duration.count() << " microseconds" << std::endl;


// start = std::chrono::high_resolution_clock::now(); 
// end = std::chrono::high_resolution_clock::now();
// duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
// std::cout << "Execution time for authentication : " << duration.count() << " microseconds" << std::endl;

// uint64_t start = rdtsc();
        
// uint64_t end = rdtsc();

// std::cout << "Execution time (in CPU cycles): " << (end - start) << " cycles\n";
