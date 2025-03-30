#include <iostream>
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

int main() {
    // Define the expected number of cycles for 1 second at 2.4 GHz (2.4 billion cycles)
    const long long expected_cycles = 2.4e9;  // 2.4 GHz * 1 second

    CycleCounter counter;

    // Start the cycle count
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

    // End the cycle count
    long long end = counter.getCycles();

    // Calculate the number of CPU cycles during the workload
    long long measured_cycles = end - start;
    std::cout << "Measured CPU cycles: " << measured_cycles << " cycles" << std::endl;

    // Compare with the expected cycles for 1 second at 2.4 GHz
    std::cout << "Expected CPU cycles for 1 second (2.4 GHz): " << expected_cycles << " cycles" << std::endl;
    std::cout << "Difference: " << (measured_cycles - expected_cycles) << " cycles" << std::endl;

    return 0;
}
