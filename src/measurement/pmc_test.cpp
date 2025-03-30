#include <iostream>
#include <fstream>
#include <string>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <chrono>

class CycleCounter {
private:
    int fddev;

public:
    CycleCounter() {
        struct perf_event_attr attr;
        memset(&attr, 0, sizeof(attr));
        attr.type = PERF_TYPE_HARDWARE;
        attr.config = PERF_COUNT_HW_CPU_CYCLES;

        // Open the performance counter
        fddev = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
        if (fddev == -1) {
            perror("perf_event_open failed");
            exit(EXIT_FAILURE);
        }
    }

    ~CycleCounter() {
        close(fddev);  // Cleanup when object goes out of scope
    }

    long long getCycles() {
        long long cycles = 0;
        if (read(fddev, &cycles, sizeof(cycles)) < (ssize_t)sizeof(cycles)) {
            perror("Failed to read CPU cycles");
            return 0;
        }
        return cycles;
    }
};

// Function to read the CPU frequency from /proc/cpuinfo
double getCpuFrequency() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    double frequency = 0.0;

    while (std::getline(cpuinfo, line)) {
        if (line.find("cpu MHz") != std::string::npos) {
            // Extract the frequency (in MHz) from the line
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                frequency = std::stod(line.substr(pos + 1));
                break;
            }
        }
    }

    return frequency / 1000.0;  // Convert MHz to GHz
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

    long long start = counter.getCycles();

    // Create a known workload that runs for 1 second
    auto start_time = std::chrono::high_resolution_clock::now();
    while (true) {
        auto now_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now_time - start_time;
        if (elapsed.count() >= 1.0) {  // Run for 1 second
            break;
        }
    }

    long long end = counter.getCycles();

    std::cout << "Measured CPU cycles: " << (end - start) << " cycles" << std::endl;
    std::cout << "Difference from expected cycles: " << (end - start - expected_cycles) << " cycles" << std::endl;

    // Calculate the time difference in microseconds
    long long cycle_difference = end - start;
    double time_in_seconds = cycle_difference / (cpuFrequencyGHz * 1e9);  // Time in seconds
    double time_in_microseconds = time_in_seconds * 1e6;  // Convert time from seconds to microseconds

    std::cout << "Time difference (in microseconds): " << time_in_microseconds << " microseconds" << std::endl;
    
    return 0;
}
