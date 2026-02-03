#pragma once

#include <memory>
#include "infrastructure/window/Window.hpp"
#include "application/Session.hpp"
#include "ui/UI.hpp"
#include "ui/views/Hybrid2DView.hpp"

namespace SisterSTRATA {

class Application {
public:
    struct Config {
        bool useHybridMode = false;
    };
    Application();
    Application(const Config& config);
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
    std::unique_ptr<::UI::Views::Hybrid2DView> hybridView_;
    Config config_;
};

} // namespace SisterSTRATA
