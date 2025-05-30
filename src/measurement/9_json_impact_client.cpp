/**
 * @file 9_json_impact_client.cpp
 * @brief This file's goal is to measure the overheads of using JSON as a serialization method.
 * 
 */
#include <thread>

#include "../UAV.hpp"
#include "../utils.hpp"
#include "../CycleCounter.hpp"

void warmup(UAV * A){
    // unsigned char rand[PUF_SIZE];
    // generate_random_bytes(rand);
    // unsigned char out[PUF_SIZE];
    // A->callPUF(rand, out);
    // print_hex(out, PUF_SIZE);

    json msg = {{"id", "A"}};

    A->socketModule.sendMessage(msg);

}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No IP address provided. Please provide the IP as an argument." << std::endl;
        return 1;  // Exit with an error code
    }

    const char* ip = argv[1];  // Read IP from command-line argument 
    
    CycleCounter counter;
    
    // Test a PUF computation t ocompare the two rpi
    UAV A = UAV("A");
    A.socketModule.initiateConnection(ip, 8080);
    
    if (argc > 2 && std::string(argv[2]) == "--warmup") {
        warmup(&A);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }    
    
    long long start = counter.getCycles();
    
    json msg = {{"id", "A"}};
    A.socketModule.sendMessage(msg);

    long long end = counter.getCycles();
    long long cycle_difference = end - start;
    
    std::cout << "Measured CPU cycles: " << cycle_difference << " cycles" << std::endl;
    A.socketModule.closeConnection();
    
    return 0;
}
