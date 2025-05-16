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
#if defined(__linux__)
#include <linux/perf_event.h>
#include <sys/syscall.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <winnt.h>
#include <winbase.h>
#include <processthreadsapi.h>
#include <io.h>
#endif
#include "lib/unistd.h"
#include <fcntl.h>
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