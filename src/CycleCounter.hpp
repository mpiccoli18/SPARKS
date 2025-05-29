/**
 * @file CycleCounter.hpp
 * @brief CycleCounter class header
 * 
 * This file holds the CycleCounter class header.
 * 
 */

#ifndef CYCLECOUNTER_HPP
#define CYCLECOUNTER_HPP

#include <iostream>
#include <fstream>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

/// @brief This class implements the cycles counter
class CycleCounter {
    private:
        int fddev;
        int cycleCount;
    
    public:
        CycleCounter();
    
        ~CycleCounter();

        void start();
        void stop();
        void reset();
        int cycles() const;
        long long getCycles();
};

#endif