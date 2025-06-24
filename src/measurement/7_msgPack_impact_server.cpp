/**
 * @file 7_json_impact_server.cpp
 * @brief This file's goal is to measure the overheads of using JSON as a serialization method.
 * 
 */
#include <thread>

#include "../UAV.hpp"
#include "../utils.hpp"
#include "../CycleCounter.hpp"

int main() {

    
    CycleCounter counter;
    
    // Test a PUF computation t ocompare the two rpi
    UAV A("A");
    
    A.socketModule.waitForConnection(8080);
    std::unordered_map<std::string, std::string> rsp;
    rsp.reserve(1);
    A.socketModule.receiveMsg(rsp);
    printMsg(rsp);

    A.socketModule.closeConnection();

    return 0;
}
