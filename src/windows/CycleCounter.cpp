/**
 * @file CycleCounter.cpp
 * @brief CycleCounter class implementation
 * 
 * This file holds the CycleCounter class implementation.
 * 
 */

#include "CycleCounter.hpp"
#include <iostream>
/// @brief Constructor
CycleCounter::CycleCounter() {
    startCycles = __rdtsc();
}
/// @brief Destructor
CycleCounter::~CycleCounter() {
    unsigned __int64 endCycles = __rdtsc();
    std::cout << "CPU cycles elapsed: " << (endCycles - startCycles) << "\n";
}

/// @brief Read the current CPU cycle count
/// @return The number of CPU cycles since the counter was opened
long long CycleCounter::getCycles() {
    return static_cast<long long>(__rdtsc());
}
