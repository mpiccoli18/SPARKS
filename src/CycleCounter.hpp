/**
 * @file CycleCounter.hpp
 * @brief CycleCounter class header
 * 
 * This file holds the CycleCounter class header.
 * 
 */

#ifndef CYCLECOUNTER_HPP
#define CYCLECOUNTER_HPP

#include <fstream>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <cstring>

/// @brief This class implements the cycles counter
class CycleCounter {
    private:
        int fddev;
    
    public:
        CycleCounter();
    
        ~CycleCounter();

        long long getCycles();
};

#endif