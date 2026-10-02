// Entry point. Phase 1 only prints the version; the UCI loop is added in Phase 2.
#include <iostream>

// CMake injects the real version; the fallback keeps the file compilable on its own.
#ifndef GAMBIT_VERSION
#define GAMBIT_VERSION "unknown"
#endif

int main() {
    // std::endl flushes, so the line also appears when stdout is a pipe.
    std::cout << "Gambit " << GAMBIT_VERSION << std::endl;
    return 0;
}
