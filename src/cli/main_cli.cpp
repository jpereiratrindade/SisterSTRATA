#include "application/Session.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    std::cout << "[sister_strata_cli] Starting Headless Mode..." << std::endl;

    try {
        // 1. Initialize Session
        Application::Session session;
        std::cout << "[sister_strata_cli] Session initialized." << std::endl;

        // 2. Load File if arg provided
        if (argc > 1) {
            std::string path = argv[1];
            std::cout << "[sister_strata_cli] Loading file: " << path << std::endl;
            session.loadWorld(path);
            
            // 3. Perform Analysis (Example: Impact Profile)
            std::cout << "[sister_strata_cli] Generating Impact Profile..." << std::endl;
            std::string profile = session.generateImpactProfileText();
            std::cout << "\n--- IMPACT PROFILE ---\n" << profile << "\n----------------------\n" << std::endl;
        } else {
            std::cout << "Usage: sister_strata_cli <path_to_world_file>" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "[sister_strata_cli] Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
