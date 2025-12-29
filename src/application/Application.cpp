#include "application/Application.hpp"
#include "world3d/World3D.hpp"
#include "application/dtos/UIData.hpp"
#include <iostream>
#include <SDL2/SDL.h>

namespace SisterSTRATA {

Application::Application() {
    init();
}

Application::~Application() {
    shutdown();
}

void Application::init() {
    std::cout << "[Application] Initializing..." << std::endl;

    // 1. Initialize Window
    window_ = std::make_unique<Infrastructure::Windowing::Window>("SisterSTRATA - Scientific Engine for Layered Landscapes", 1280, 720);

    // 2. Initialize World3D (Vulkan)
    // Critical: Must be done before UI because UI depends on Vulkan Context
    World3D::init(window_->getNativeWindow());

    // 3. Initialize UI
    ::UI::VulkanInitInfo info;
    info.instance = World3D::getInstance();
    info.physicalDevice = World3D::getPhysicalDevice();
    info.device = World3D::getDevice();
    info.queue = World3D::getGraphicsQueue();
    info.queueFamily = World3D::getGraphicsQueueFamilyIndex();
    info.descriptorPool = World3D::getDescriptorPool();
    info.renderPass = World3D::getRenderPass();
    info.minImageCount = World3D::getMinImageCount();
    info.imageCount = World3D::getImageCount();

    ui_ = std::make_unique<::UI::UserInterface>();
    
    // Bind UI Callbacks
    ui_->onLoadDemo = []() { World3D::loadDemoCloud(); };
    ui_->onOpenFile = [](std::string path) { World3D::loadFile(path); };
    ui_->onSaveFile = [](std::string path) { World3D::saveFile(path); }; // Bind
    ui_->onCloseFile = []() { World3D::clear(); }; 
    ui_->onExit = [this]() { this->running_ = false; };

    ui_->init(window_->getNativeWindow(), info);

    // 4. Initialize Session
    session_ = std::make_unique<::Application::Session>();
    
    std::cout << "[Application] Initialization Complete." << std::endl;
}

void Application::shutdown() {
    if (window_) {
        // Ensure GPU is idle before destroying resources
        World3D::getDevice().waitIdle();
        
        ui_->shutdown();
        World3D::shutdown();
        
        ui_.reset();
        window_.reset();
        session_.reset();
    }
}

void Application::run() {
    mainLoop();
}

void Application::mainLoop() {
    static Uint64 lastTime = SDL_GetPerformanceCounter();

    while (running_ && !window_->shouldClose()) {
        Uint64 now = SDL_GetPerformanceCounter();
        float deltaTime = (float)(now - lastTime) / SDL_GetPerformanceFrequency();
        lastTime = now;

        processEvents();

        // --- Render Phase ---
        
        // 1. Prepare 3D Frame
        World3D::beginFrame();
        
        // 2. Update UI Logic
        ui_->beginFrame();
        
        ::Application::DTO::UIData uiData;
        uiData.framerate = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
        uiData.frameTimeMs = deltaTime * 1000.0f;
        uiData.startMessage = "System Ready";
        
        ui_->draw(uiData);
        
        // 3. Present (Render 3D + UI Overlay)
        World3D::endFrame([this](vk::CommandBuffer cmd) {
            this->ui_->render(cmd);
        });
        
        ui_->endFrame();
    }
}

void Application::processEvents() {
    window_->pollEvents([this](const SDL_Event& event) {
        // 1. Pass to UI first
        ui_->processEvent(&event);
        
        // 2. Determine if 3D World should receive input
        bool uiMouse = ui_->wantsToCaptureMouse();
        bool uiKeyboard = ui_->wantsToCaptureKeyboard();

        // 3. Input Routing Logic
        bool isMouseEvent = (event.type == SDL_MOUSEMOTION || 
                           event.type == SDL_MOUSEBUTTONDOWN || 
                           event.type == SDL_MOUSEBUTTONUP || 
                           event.type == SDL_MOUSEWHEEL);
                           
        bool isKeyboardEvent = (event.type == SDL_KEYDOWN || 
                              event.type == SDL_KEYUP);

        if (isMouseEvent && uiMouse) return;
        
        // Allow keyboard shortcuts (WASD) unless text input is active
        if (isKeyboardEvent && ui_->wantsTextInput()) return;

        // 4. Pass to World3D
        World3D::processEvent(event);
    });
}


} // namespace SisterSTRATA
