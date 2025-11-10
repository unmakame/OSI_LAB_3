#include "parentProcess.hpp"
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

ParentProcess::ParentProcess(const std::string& file1, const std::string& file2) 
    : shm("lab3_shm", 1024, true),
      dataReadySem1("lab3_data_ready1", true), 
      dataReadySem2("lab3_data_ready2", true),
      dataProcessedSem1("lab3_data_processed1", true), 
      dataProcessedSem2("lab3_data_processed2", true),
      mutexSem("lab3_mutex", true),
      filename1(file1), 
      filename2(file2) {}

#ifdef WINDOWS_PLATFORM

bool ParentProcess::createChildProcess(const std::string& executable, const std::string& filename, 
                                     const std::string& processNumber) {
    std::string commandLine = executable + " " + filename + " " + processNumber;
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    char* cmdLine = new char[commandLine.length() + 1];
    strcpy(cmdLine, commandLine.c_str());
    
    BOOL success = CreateProcessA(
        NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi
    );
    
    delete[] cmdLine;
    
    if (success) {
        childProcesses.push_back(pi);
        return true;
    }
    
    std::cerr << "CreateProcess failed (" << GetLastError() << ")" << std::endl;
    return false;
}

#else

bool ParentProcess::createChildProcess(const std::string& executable, const std::string& filename, 
                                     const std::string& processNumber) {
    pid_t pid = fork();
    
    if (pid == 0) {
        execl(executable.c_str(), executable.c_str(), filename.c_str(), processNumber.c_str(), nullptr);
        perror("execl failed");
        exit(1);
    } else if (pid > 0) {
        childPids.push_back(pid);
        return true;
    }
    
    perror("fork failed");
    return false;
}

#endif

bool ParentProcess::initializeSharedMemory() {
    std::cout << "Initializing shared resources..." << std::endl;
    
    bool shm_ok = shm.createSharedMemory();
    bool sem1_ok = dataReadySem1.createSemaphore(0);
    bool sem2_ok = dataReadySem2.createSemaphore(0);
    bool sem3_ok = dataProcessedSem1.createSemaphore(0);
    bool sem4_ok = dataProcessedSem2.createSemaphore(0);
    bool mutex_ok = mutexSem.createSemaphore(1);
    
    if (shm_ok && sem1_ok && sem2_ok && sem3_ok && sem4_ok && mutex_ok) {
        std::cout << "All shared resources initialized successfully!" << std::endl;
        return true;
    } else {
        std::cerr << "Failed to initialize shared resources!" << std::endl;
        return false;
    }
}

bool ParentProcess::createChildProcesses() {
    bool success1 = createChildProcess("./childProcess", filename1, "1");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    bool success2 = createChildProcess("./childProcess", filename2, "2");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    if (success1 && success2) {
        std::cout << "Both child processes created successfully!" << std::endl;
    }
    
    return success1 && success2;
}

void ParentProcess::sendToChild(const std::string& data, int childNumber) {
    mutexSem.waitSemaphore();
    
    std::string message = std::to_string(childNumber) + ":" + data;
    shm.writeToSharedMemory(message);
    
    mutexSem.postSemaphore();
    
    if (childNumber == 1) {
        dataReadySem1.postSemaphore();
        dataProcessedSem1.waitSemaphore();
    } else {
        dataReadySem2.postSemaphore();
        dataProcessedSem2.waitSemaphore();
    }
}

void ParentProcess::run() {
    std::cout << "Parent process started. Enter strings (empty line to exit):" << std::endl;
    
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (input.empty()) {
            break;
        }

        int targetChild = StringProcessor::shouldGoToPipe2(input) ? 2 : 1;
        std::cout << "Routing to Child" << targetChild 
                  << (targetChild == 2 ? " (long string >10): " : " (short string ≤10): ") 
                  << input << std::endl;
        
        sendToChild(input, targetChild);
    }

    sendToChild("EXIT", 1);
    sendToChild("EXIT", 2);
    
    std::cout << "Waiting for child processes to finish..." << std::endl;
    cleanup();
    std::cout << "Parent process finished." << std::endl;
}

void ParentProcess::cleanup() {
#ifdef WINDOWS_PLATFORM
    for (auto& pi : childProcesses) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    childProcesses.clear();
#else
    for (auto pid : childPids) {
        waitpid(pid, nullptr, 0);
    }
    childPids.clear();
#endif
}

ParentProcess::~ParentProcess() {
    cleanup();
}
