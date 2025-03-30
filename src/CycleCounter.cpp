#include "CycleCounter.hpp"

CycleCounter::CycleCounter() {
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
    
CycleCounter::~CycleCounter() {
    close(fddev);  // Cleanup when object goes out of scope
}
    
long long CycleCounter::getCycles() {
    long long cycles = 0;
    if (read(fddev, &cycles, sizeof(cycles)) < (ssize_t)sizeof(cycles)) {
        perror("Failed to read CPU cycles");
        return 0;
    }
    return cycles;
}
