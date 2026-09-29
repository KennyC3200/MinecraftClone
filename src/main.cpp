#include "core/Engine.hpp"

#include <exception>
#include <iostream>

#if defined(_WIN32) && defined(MC_PREFER_DGPU)
// Run on NVIDIA GPU
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
}
#endif

int main() {
    try {
        mcc::Engine engine;
        engine.Run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
