#include <iostream>
#include <string>
#include "parentProcess.hpp"

int main() {
    std::string filename1, filename2;
    
    std::cout << "Enter filename for child1: ";
    std::getline(std::cin, filename1);
    
    std::cout << "Enter filename for child2: ";
    std::getline(std::cin, filename2);
    
    if (filename1.empty() || filename2.empty()) {
        std::cerr << "Filenames cannot be empty!" << std::endl;
        return 1;
    }

    ParentProcess parent(filename1, filename2);
    
    if (!parent.initializeSharedMemory()) {
        std::cerr << "Failed to initialize shared memory!" << std::endl;
        return 1;
    }
    
    if (!parent.createChildProcesses()) {
        std::cerr << "Failed to create child processes!" << std::endl;
        return 1;
    }
    
    parent.run();
    
    return 0;
}