#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "sharedMemoryManager.hpp"
#include "syncManager.hpp"
#include "stringProcessor.hpp"
#include "platform.hpp"

#ifdef WINDOWS_PLATFORM
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/wait.h>
#endif

class ParentProcess {
private:
    SharedMemoryManager shm;
    SyncManager dataReadySem1, dataReadySem2;
    SyncManager dataProcessedSem1, dataProcessedSem2;
    SyncManager mutexSem;
    std::string filename1, filename2;
    
#ifdef WINDOWS_PLATFORM
    std::vector<PROCESS_INFORMATION> childProcesses;
#else
    std::vector<pid_t> childPids;
#endif

    bool createChildProcess(const std::string& executable, const std::string& filename, 
                          const std::string& processNumber);

public:
    ParentProcess(const std::string& file1, const std::string& file2);
    ~ParentProcess();

    bool initializeSharedMemory();
    bool createChildProcesses();
    void run();
    void cleanup();
    void sendToChild(const std::string& data, int childNumber);
};