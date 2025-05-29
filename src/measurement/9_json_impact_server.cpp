/**
 * @file 9_json_impact_server.cpp
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

int main() {

    
    CycleCounter counter;
    
    // Test a PUF computation t ocompare the two rpi
    UAV A = UAV("A");
    
    A.socketModule.waitForConnection(8080);

    json rsp = A.socketModule.receiveMessage();
    printJSON(rsp);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    rsp = A.socketModule.receiveMessage();
    printJSON(rsp);

    A.socketModule.closeConnection();

    return 0;
}
