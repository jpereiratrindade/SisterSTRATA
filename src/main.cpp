#include <iostream>
#include "application/Application.hpp"

int main() {
    try {
        SisterPEC::Application app;
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
