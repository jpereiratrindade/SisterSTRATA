#pragma once

#include <memory>
#include "infrastructure/window/Window.hpp"
#include "application/Session.hpp"
#include "ui/UI.hpp"

namespace SisterPEC {

class Application {
public:
    Application();
    ~Application();

    void run();

private:
    void init();
    void mainLoop();
    void shutdown();
    void processEvents();

    bool running_ = true;

    // Systems
    std::unique_ptr<Infrastructure::Windowing::Window> window_;
    std::unique_ptr<::Application::Session> session_; // Use fully qualified or using declaration
    std::unique_ptr<::UI::UserInterface> ui_;
};

} // namespace SisterPEC
