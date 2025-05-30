/**
 * @file 8_warmup_impact.cpp
 * @brief This file's goal is to measure the impact of the warmup stage on the overheads of several functions.
 * 
 */

#include "../UAV.hpp"
#include "../utils.hpp"
#include "../CycleCounter.hpp"

void warmup(UAV * A){
    unsigned char rand[PUF_SIZE];
    generate_random_bytes(rand);
    unsigned char out[PUF_SIZE];
    A->callPUF(rand, out);
    print_hex(out, PUF_SIZE);
}

int main(int argc, char* argv[]) {

    
    CycleCounter counter;
    
    // Test a PUF computation t ocompare the two rpi
    UAV A = UAV("A");
    
    if (argc > 1 && std::string(argv[1]) == "--warmup") {
        warmup(&A);
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
