/**
 * @file CycleCounter.cpp
 * @brief CycleCounter class implementation
 * 
 * This file holds the CycleCounter class implementation.
 * 
 */

#include "CycleCounter.hpp"

/// @brief Constructor
CycleCounter::CycleCounter() {
    struct perf_event_attr attr;
    memset(&attr, 0, sizeof(attr)); // Zero-initialize the structure

    attr.type = PERF_TYPE_HARDWARE;           // We want to count hardware events
    attr.config = PERF_COUNT_HW_CPU_CYCLES;   // Specifically, CPU cycles

    // Open the performance counter using a syscall
    // Parameters:
    // - attr: pointer to the event attributes
    // - pid: 0 = current process
    // - cpu: -1 = all CPUs
    // - group_fd: -1 = no group
    // - flags: 0 = no special flags
    fddev = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
    
    if (fddev == -1) {
        perror("perf_event_open failed");  // Print error message
        exit(EXIT_FAILURE);               // Exit program on failure
    }
}

/// @brief Destructor
CycleCounter::~CycleCounter() {
    close(fddev);  // Close the file descriptor when the object is destroyed
}

/// @brief Read the current CPU cycle count
/// @return The number of CPU cycles since the counter was opened
long long CycleCounter::getCycles() {
    long long cycles = 0;

    // Read the current value from the performance counter
    if (read(fddev, &cycles, sizeof(cycles)) < (ssize_t)sizeof(cycles)) {
        perror("Failed to read CPU cycles");  // Error if read fails
        return 0;  // Return 0 if unable to read
    }

    return cycles;  // Return the number of cycles read
}
