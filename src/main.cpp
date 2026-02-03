#include <iostream>
#include "application/Application.hpp"

int main(int argc, char** argv) {
    try {
        SisterSTRATA::Application::Config config;
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--hybrid" || arg == "-d" || arg == "--no-gpu" || arg == "--cpu") { // Category D, or explicit No-GPU
                config.useHybridMode = true;
            }
        }
        
        SisterSTRATA::Application app(config);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Fatal Error: Unknown exception" << std::endl;
        return 1;
    }

    return 0;
}
