#ifndef CYCLECOUNTER_HPP
#define CYCLECOUNTER_HPP

#include <fstream>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

class CycleCounter {
    private:
        int fddev;
    
    public:
        CycleCounter();
    
        ~CycleCounter();

        long long getCycles();
};

#endif