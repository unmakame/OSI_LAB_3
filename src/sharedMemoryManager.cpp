#include "sharedMemoryManager.hpp"
#include <iostream>
#include <cstring>

#ifdef WINDOWS_PLATFORM

SharedMemoryManager::SharedMemoryManager(const std::string& name, size_t size, bool create) 
    : memorySize(size), memoryName(name), isCreator(create), hMapFile(NULL), mappedMemory(NULL) {}

SharedMemoryManager::~SharedMemoryManager() {
    closeSharedMemory();
}

bool SharedMemoryManager::createSharedMemory() {
    hMapFile = CreateFileMappingA(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, memorySize, memoryName.c_str());
    
    if (hMapFile == NULL) {
        std::cerr << "CreateFileMapping failed: " << GetLastError() << std::endl;
        return false;
    }

    mappedMemory = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, memorySize);
    if (mappedMemory == NULL) {
        std::cerr << "MapViewOfFile failed: " << GetLastError() << std::endl;
        CloseHandle(hMapFile);
        hMapFile = NULL;
        return false;
    }

    return true;
}

bool SharedMemoryManager::openSharedMemory() {
    hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, memoryName.c_str());
    if (hMapFile == NULL) {
        std::cerr << "OpenFileMapping failed: " << GetLastError() << std::endl;
        return false;
    }

    mappedMemory = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, memorySize);
    if (mappedMemory == NULL) {
        std::cerr << "MapViewOfFile failed: " << GetLastError() << std::endl;
        CloseHandle(hMapFile);
        hMapFile = NULL;
        return false;
    }

    return true;
}

bool SharedMemoryManager::writeToSharedMemory(const std::string& data, size_t offset) {
    if (mappedMemory == NULL || offset + data.length() >= memorySize) return false;
    
    char* dest = static_cast<char*>(mappedMemory) + offset;
    strncpy(dest, data.c_str(), memorySize - offset - 1);
    dest[data.length()] = '\0';
    
    return true;
}

bool SharedMemoryManager::readFromSharedMemory(std::string& data, size_t offset, size_t length) {
    if (mappedMemory == NULL || offset >= memorySize) return false;
    
    char* src = static_cast<char*>(mappedMemory) + offset;
    if (length == 0) {
        data = std::string(src);
    } else {
        data = std::string(src, length);
    }
    
    return true;
}

void SharedMemoryManager::closeSharedMemory() {
    if (mappedMemory != NULL) {
        UnmapViewOfFile(mappedMemory);
        mappedMemory = NULL;
    }
    if (hMapFile != NULL) {
        CloseHandle(hMapFile);
        hMapFile = NULL;
    }
}

bool SharedMemoryManager::isOpen() const {
    return mappedMemory != NULL;
}

#else

SharedMemoryManager::SharedMemoryManager(const std::string& name, size_t size, bool create) 
    : memorySize(size), memoryName("/" + name), isCreator(create), shm_fd(-1), mappedMemory(NULL) {}

SharedMemoryManager::~SharedMemoryManager() {
    closeSharedMemory();
}

bool SharedMemoryManager::createSharedMemory() {
    shm_fd = shm_open(memoryName.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open create");
        return false;
    }

    if (ftruncate(shm_fd, memorySize) == -1) {
        perror("ftruncate");
        close(shm_fd);
        shm_fd = -1;
        return false;
    }

    mappedMemory = mmap(NULL, memorySize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (mappedMemory == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        shm_fd = -1;
        return false;
    }

    return true;
}

bool SharedMemoryManager::openSharedMemory() {
    shm_fd = shm_open(memoryName.c_str(), O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open open");
        return false;
    }

    mappedMemory = mmap(NULL, memorySize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (mappedMemory == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        shm_fd = -1;
        return false;
    }

    return true;
}

bool SharedMemoryManager::writeToSharedMemory(const std::string& data, size_t offset) {
    if (mappedMemory == NULL || offset + data.length() >= memorySize) return false;
    
    char* dest = static_cast<char*>(mappedMemory) + offset;
    strncpy(dest, data.c_str(), memorySize - offset - 1);
    dest[data.length()] = '\0';
    
    return true;
}

bool SharedMemoryManager::readFromSharedMemory(std::string& data, size_t offset, size_t length) {
    if (mappedMemory == NULL || offset >= memorySize) return false;
    
    char* src = static_cast<char*>(mappedMemory) + offset;
    if (length == 0) {
        data = std::string(src);
    } else {
        data = std::string(src, length);
    }
    
    return true;
}

void SharedMemoryManager::closeSharedMemory() {
    if (mappedMemory != NULL) {
        munmap(mappedMemory, memorySize);
        mappedMemory = NULL;
    }
    if (shm_fd != -1) {
        close(shm_fd);
        shm_fd = -1;
    }
    if (isCreator) {
        shm_unlink(memoryName.c_str());
    }
}

bool SharedMemoryManager::isOpen() const {
    return mappedMemory != NULL;
}

#endif