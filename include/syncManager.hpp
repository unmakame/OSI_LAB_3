#pragma once

#include <string>

#ifdef WINDOWS_PLATFORM
    #include <windows.h>
#else
    #include <semaphore.h>
#endif

class SyncManager {
private:
#ifdef WINDOWS_PLATFORM
    HANDLE semaphore;
#else
    sem_t* semaphore;
#endif
    std::string semName;
    bool isCreator;

public:
    SyncManager(const std::string& name, bool create = false);
    ~SyncManager();
    
    bool createSemaphore(int initialValue = 1);
    bool openSemaphore();
    bool waitSemaphore();
    bool postSemaphore();
    void closeSemaphore();
};