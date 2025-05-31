/**
 * @file 6_warmup_impact.cpp
 * @brief This file's goal is to measure the impact of the warmup stage on the overheads of several functions.
 * 
 */

#include "../UAV.hpp"
#include "../utils.hpp"
#include "../CycleCounter.hpp"


int main(int argc, char* argv[]) {

    
    CycleCounter counter;
    
    // Test a PUF computation t ocompare the two rpi
    UAV A("A");
    
    if (argc > 1 && std::string(argv[1]) == "--warmup") {
        warmup();
    }    
    
    long long start = counter.getCycles();
    unsigned char x[PUF_SIZE];
    generate_random_bytes(x);
    unsigned char C[PUF_SIZE];
    
    
    A.callPUF(x,C);
    
    long long end = counter.getCycles();

    
    long long cycle_difference = end - start;

    std::cout << "Measured CPU cycles: " << cycle_difference << " cycles" << std::endl;
    
    return 0;
}
