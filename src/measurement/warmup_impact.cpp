#include <chrono>

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

int main() {
    // Get CPU frequency from user input or auto-detect
    double cpuFrequencyGHz;
    std::cout << "Enter CPU frequency in GHz (leave empty for auto-detection): ";
    std::string input;
    std::getline(std::cin, input);

    if (input.empty()) {
        // Get the CPU frequency in GHz (use getCpuFrequency function)
        cpuFrequencyGHz = getCpuFrequency();
        std::cout << "Detected CPU frequency: " << cpuFrequencyGHz << " GHz" << std::endl;

    } 
    else {
        // Parse user input if provided
        try {
            cpuFrequencyGHz = std::stod(input);
        } catch (const std::exception& e) {
            std::cerr << "Invalid input. Please enter a valid number." << std::endl;
            return EXIT_FAILURE;
        }
    }
    
    // Calculate the expected number of cycles for 1 second
    const long long expected_cycles = static_cast<long long>(cpuFrequencyGHz * 1e9);  // 1 second at the given frequency

    std::cout << "Expected CPU cycles for 1 second at " << cpuFrequencyGHz << " GHz: " << expected_cycles << " cycles" << std::endl;

    CycleCounter counter;

    
    // Create a known workload that runs for 1 second
    // auto start_time = std::chrono::high_resolution_clock::now();
    // while (true) {
    //     auto now_time = std::chrono::high_resolution_clock::now();
    //     std::chrono::duration<double> elapsed = now_time - start_time;
    //     if (elapsed.count() >= 1.0) {  // Run for 1 second
    //         break;
    //     }
    // }
    
    // Test a PUF computation t ocompare the two rpi
    UAV A = UAV("A");

    #ifdef WARMUP
    warmup(&A);
    #endif
    
    long long start = counter.getCycles();
    unsigned char x[PUF_SIZE];
    generate_random_bytes(x);
    unsigned char C[PUF_SIZE];
    
    
    A.callPUF(x,C);
    
    long long end = counter.getCycles();

    
    long long cycle_difference = end - start;

    std::cout << "Measured CPU cycles: " << cycle_difference << " cycles" << std::endl;
    
    // Calculate the time difference in microseconds
    double time_in_seconds = cycle_difference / (cpuFrequencyGHz * 1e9);  // Time in seconds
    double time_in_microseconds = time_in_seconds * 1e6;  // Convert time from seconds to microseconds
    
    std::cout << "Time (in microseconds): " << time_in_microseconds << " microseconds" << std::endl;
    
    // Calculate the distance with expected in microseconds
    long long cycle_difference_dist = end - start - expected_cycles;
    time_in_seconds = cycle_difference_dist / (cpuFrequencyGHz * 1e9);  // Time in seconds
    time_in_microseconds = time_in_seconds * 1e6;  // Convert time from seconds to microseconds
    
    std::cout << "Distance from expected cycles: " << cycle_difference_dist << " cycles" << std::endl;
    std::cout << "Distance from expected cycles: " << time_in_microseconds << " microseconds" << std::endl;
    
    return 0;
}
