#include "platform.hpp"
#include <thread>
#include <chrono>

void Platform::sleep(int milliseconds) {
#ifdef WINDOWS_PLATFORM
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}