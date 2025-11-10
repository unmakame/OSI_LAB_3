#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "stringProcessor.hpp"
#include "sharedMemoryManager.hpp"
#include "syncManager.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <filename> <process_number>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    int processNumber = std::stoi(argv[2]);
    
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return 1;
    }

    SharedMemoryManager shm("lab3_shm", 1024, false);
    SyncManager dataReadySem1("lab3_data_ready1", false);
    SyncManager dataReadySem2("lab3_data_ready2", false);
    SyncManager dataProcessedSem1("lab3_data_processed1", false);
    SyncManager dataProcessedSem2("lab3_data_processed2", false);
    SyncManager mutexSem("lab3_mutex", false);

    if (!shm.openSharedMemory() || !dataReadySem1.openSemaphore() || 
        !dataReadySem2.openSemaphore() || !dataProcessedSem1.openSemaphore() ||
        !dataProcessedSem2.openSemaphore() || !mutexSem.openSemaphore()) {
        std::cerr << "Child" << processNumber << ": Failed to open shared resources" << std::endl;
        return 1;
    }

    std::cout << "Child" << processNumber << " started successfully" << std::endl;

    SyncManager& myDataReady = (processNumber == 1) ? dataReadySem1 : dataReadySem2;
    SyncManager& myDataProcessed = (processNumber == 1) ? dataProcessedSem1 : dataProcessedSem2;

    while (true) {
        if (!myDataReady.waitSemaphore()) {
            break;
        }

        mutexSem.waitSemaphore();
        
        std::string message;
        if (!shm.readFromSharedMemory(message)) {
            mutexSem.postSemaphore();
            break;
        }

        size_t colonPos = message.find(':');
        if (colonPos == std::string::npos) {
            mutexSem.postSemaphore();
            myDataProcessed.postSemaphore();
            continue;
        }

        int targetProcess = std::stoi(message.substr(0, colonPos));
        std::string input = message.substr(colonPos + 1);

        if (targetProcess != processNumber) {
            mutexSem.postSemaphore();
            myDataProcessed.postSemaphore();
            continue;
        }

        if (input == "EXIT") {
            mutexSem.postSemaphore();
            myDataProcessed.postSemaphore();
            break;
        }

        std::string processed = StringProcessor::removeVowels(input);
        
        file << "Child" << processNumber << " processed: '" << input 
             << "' -> '" << processed << "'" << std::endl;
        file.flush();

        std::cout << "Child" << processNumber << " processed: '" << input 
                  << "' -> '" << processed << "'" << std::endl;

        mutexSem.postSemaphore();
        myDataProcessed.postSemaphore();
    }

    file.close();
    std::cout << "Child" << processNumber << " finished" << std::endl;
    return 0;
}