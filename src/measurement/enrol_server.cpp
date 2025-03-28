#include <string>
#include <chrono> 
#include <thread>

#include "../UAV.hpp"
#include "../puf.hpp"
#include "../utils.hpp"
#include "../SocketModule.hpp"

std::string idA = "A";
std::string idB = "B";
bool server = false;
std::chrono::time_point<std::chrono::high_resolution_clock> start;
std::chrono::time_point<std::chrono::high_resolution_clock> end;
std::chrono::microseconds m1;
std::chrono::microseconds m2;
std::chrono::microseconds m3;
std::chrono::microseconds m4;
std::chrono::microseconds m5;
std::chrono::microseconds m6;
std::chrono::microseconds m7;


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
    start = std::chrono::high_resolution_clock::now();
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
    end = std::chrono::high_resolution_clock::now();
    m1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    start = std::chrono::high_resolution_clock::now();
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
    end = std::chrono::high_resolution_clock::now();
    m2 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    start = std::chrono::high_resolution_clock::now();
    A->getUAVData(idB)->setR(RB);
    end = std::chrono::high_resolution_clock::now();
    m3 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // std::cout << "\nB is enroled to A\n";
   
    return 0;
}

int enrolment_client_2(UAV * A){
    start = std::chrono::high_resolution_clock::now();
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
    end = std::chrono::high_resolution_clock::now();
    m4 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    A->getUAVData(idB)->setC(CA);

    // A computes RA
    unsigned char RA[PUF_SIZE];
    A->callPUF(CA, RA);
    // std::cout << "RA : "; print_hex(RA, PUF_SIZE);

    end = std::chrono::high_resolution_clock::now();
    m5 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // A sends RA
    json msg = {{"id", idA}, {"R", toHexString(RA, PUF_SIZE)}};
    A->socketModule.sendMessage(msg);
    // std::cout << "Sent RA.\n";

    return 0;
}

int main() {

    // Creation of the UAV

    UAV A = UAV(idA);
    A.addUAV(idB);

    std::cout << "The client drone id is : " <<A.getId() << ".\n"; 
    
    A.socketModule.waitForConnection(8080);
    
    // When the programm reaches this point, the UAV are connected
    
    // We now warm up the functions 
    warmup(&A);    
    
    std::cout << "Started enrolment one side" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    int ret = enrolment_client_2(&A);
    if (ret == 1){
        return ret;
    }
    end = std::chrono::high_resolution_clock::now();
    m6 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    start = std::chrono::high_resolution_clock::now();
    ret = enrolment_client_1(&A);
    if (ret == 1){
        return ret;
    }
    end = std::chrono::high_resolution_clock::now();
    m7 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Finished enrolment" << std::endl;

    std::cout << "m1 Elapsed CPU cycles: " << m1.count() << " microseconds" << std::endl;
    std::cout << "m2 Elapsed CPU cycles: " << m2.count() << " microseconds" << std::endl;
    std::cout << "m3 Elapsed CPU cycles: " << m3.count() << " microseconds" << std::endl;
    std::cout << "m4 Elapsed CPU cycles: " << m4.count() << " microseconds" << std::endl;
    std::cout << "m5 Elapsed CPU cycles: " << m5.count() << " microseconds" << std::endl;
    std::cout << "m6 Elapsed CPU cycles: " << m6.count() << " microseconds" << std::endl;
    std::cout << "m7 Elapsed CPU cycles: " << m6.count() << " microseconds" << std::endl;
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
