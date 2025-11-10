#pragma once

#include <string>
#include <cstddef>

#ifdef WINDOWS_PLATFORM
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

class SharedMemoryManager {
private:
#ifdef WINDOWS_PLATFORM
    HANDLE hMapFile;
#else
    int shm_fd;
#endif
    void* mappedMemory;
    size_t memorySize;
    std::string memoryName;
    bool isCreator;

public:
    SharedMemoryManager(const std::string& name, size_t size, bool create = false);
    ~SharedMemoryManager();
    
    bool createSharedMemory();
    bool openSharedMemory();
    void* getMappedMemory() const;
    bool writeToSharedMemory(const std::string& data, size_t offset = 0);
    bool readFromSharedMemory(std::string& data, size_t offset = 0, size_t length = 0);
    void closeSharedMemory();
    bool isOpen() const;
};