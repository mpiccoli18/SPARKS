/**
 * @file 7_json_impact_client.cpp
 * @brief This file's goal is to measure the overheads of using JSON as a serialization method.
 * 
 */
#include <thread>

#include "../UAV.hpp"
#include "../utils.hpp"
#include "../CycleCounter.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No IP address provided. Please provide the IP as an argument." << std::endl;
        return 1;  // Exit with an error code
    }

    const char* ip = argv[1];  // Read IP from command-line argument 
    
    CycleCounter counter;
    
    // Test a PUF computation t ocompare the two rpi
    UAV A("A");
    A.socketModule.initiateConnection(ip, 8080);
    
    if (argc > 2 && std::string(argv[2]) == "--warmup") {
        warmup();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }    

    unsigned char rnd[PUF_SIZE];
    generate_random_bytes(rnd);
    
    long long start = counter.getCycles();
    
    std::unordered_map<std::string, std::string> msg;
    msg.emplace("value",std::string(reinterpret_cast<const char*>(rnd), 32));
    A.socketModule.sendMsgPack(msg);

    long long end = counter.getCycles();
    long long cycle_difference = end - start;
    
    std::cout << "Measured CPU cycles: " << cycle_difference << " cycles" << std::endl;
    A.socketModule.closeConnection();
    
    return 0;
}
