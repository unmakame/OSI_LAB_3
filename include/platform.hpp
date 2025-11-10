#pragma once

#ifdef _WIN32
    #ifndef WINDOWS_PLATFORM
        #define WINDOWS_PLATFORM
    #endif
    #include <windows.h>
#else
    #ifndef UNIX_PLATFORM
        #define UNIX_PLATFORM
    #endif
    #include <unistd.h>
    #include <sys/wait.h>
    #include <signal.h>
    #include <sys/mman.h>
    #include <fcntl.h>
    #include <sys/stat.h>
#endif

#include <string>

class Platform {
public:
    static void sleep(int milliseconds);
};