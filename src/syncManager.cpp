#include "syncManager.hpp"
#include <iostream>

#ifdef WINDOWS_PLATFORM

SyncManager::SyncManager(const std::string& name, bool create) 
    : semName(name), isCreator(create), semaphore(NULL) {}

SyncManager::~SyncManager() {
    closeSemaphore();
}

bool SyncManager::createSemaphore(int initialValue) {
    semaphore = CreateSemaphoreA(NULL, initialValue, 1, semName.c_str());
    if (semaphore == NULL) {
        std::cerr << "CreateSemaphore failed: " << GetLastError() << std::endl;
        return false;
    }
    return true;
}

bool SyncManager::openSemaphore() {
    semaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, semName.c_str());
    if (semaphore == NULL) {
        std::cerr << "OpenSemaphore failed: " << GetLastError() << std::endl;
        return false;
    }
    return true;
}

bool SyncManager::waitSemaphore() {
    DWORD result = WaitForSingleObject(semaphore, INFINITE);
    return result == WAIT_OBJECT_0;
}

bool SyncManager::postSemaphore() {
    return ReleaseSemaphore(semaphore, 1, NULL) != FALSE;
}

void SyncManager::closeSemaphore() {
    if (semaphore != NULL) {
        CloseHandle(semaphore);
        semaphore = NULL;
    }
}

#else

SyncManager::SyncManager(const std::string& name, bool create) 
    : semName("/" + name), isCreator(create), semaphore(SEM_FAILED) {}

SyncManager::~SyncManager() {
    closeSemaphore();
}

bool SyncManager::createSemaphore(int initialValue) {
    sem_unlink(semName.c_str()); // Удаляем старый если существует
    semaphore = sem_open(semName.c_str(), O_CREAT, 0666, initialValue);
    if (semaphore == SEM_FAILED) {
        perror("sem_open create");
        return false;
    }
    return true;
}

bool SyncManager::openSemaphore() {
    semaphore = sem_open(semName.c_str(), 0);
    if (semaphore == SEM_FAILED) {
        perror("sem_open open");
        return false;
    }
    return true;
}

bool SyncManager::waitSemaphore() {
    return sem_wait(semaphore) == 0;
}

bool SyncManager::postSemaphore() {
    return sem_post(semaphore) == 0;
}

void SyncManager::closeSemaphore() {
    if (semaphore != SEM_FAILED) {
        sem_close(semaphore);
        semaphore = SEM_FAILED;
    }
    if (isCreator) {
        sem_unlink(semName.c_str());
    }
}

#endif