#include "application/Application.hpp"
#include "world3d/World3D.hpp"
#include "application/dtos/UIData.hpp"
#include "infrastructure/llm/OllamaMockAdapter.hpp"
#include "infrastructure/llm/OllamaAdapter.hpp"
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

    // 4. Initialize Session (First, so UI can link to it)
    session_ = std::make_unique<::Application::Session>();
    
    // LLM Service setup
    auto realLLM = std::make_unique<::Infrastructure::LLM::OllamaAdapter>();
    if (realLLM->isAvailable()) {
        std::cout << "[Application] Ollama detected. Using Qwen2.5:7b." << std::endl;
        session_->setLLMService(std::move(realLLM));
    } else {
        std::cout << "[Application] Ollama not found. Falling back to Mock LLM." << std::endl;
        session_->setLLMService(std::make_unique<::Infrastructure::LLM::OllamaMockAdapter>());
    }

    ui_ = std::make_unique<::UI::UserInterface>();
    
    // Bind UI Callbacks
    ui_->onLoadDemo = []() { World3D::loadDemoCloud(); };
    ui_->onOpenFile = [this](std::string path) { 
        World3D::loadFile(path); 
        // Sidecar Load
        try {
            this->session_->getNarrativeSystem().deserialize(path + ".json");
            std::cout << "[Application] Narrative data loaded from " << path << ".json" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[Application] Warning: Could not load narrative data: " << e.what() << std::endl;
        }
        try {
            this->session_->getDiscursiveSystemRepository().deserialize(path + ".discursive.json");
            std::cout << "[Application] Discursive data loaded from " << path << ".discursive.json" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[Application] Warning: Could not load discursive data: " << e.what() << std::endl;
        }
        try {
            this->session_->getRecommendationTrajectory().deserialize(path + ".recommendation.json");
            std::cout << "[Application] Recommendation data loaded from " << path << ".recommendation.json" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[Application] Warning: Could not load recommendation data: " << e.what() << std::endl;
        }
    };
    ui_->onSaveFile = [this](std::string path) { 
        if (World3D::saveFile(path)) {
            // Sidecar Save
            try {
                this->session_->getNarrativeSystem().serialize(path + ".json");
                std::cout << "[Application] Narrative data saved to " << path << ".json" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[Application] Error saving narrative data: " << e.what() << std::endl;
            }
            try {
                this->session_->getDiscursiveSystemRepository().serialize(path + ".discursive.json");
                std::cout << "[Application] Discursive data saved to " << path << ".discursive.json" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[Application] Error saving discursive data: " << e.what() << std::endl;
            }
            try {
                this->session_->getRecommendationTrajectory().serialize(path + ".recommendation.json");
                std::cout << "[Application] Recommendation data saved to " << path << ".recommendation.json" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[Application] Error saving recommendation data: " << e.what() << std::endl;
            }
        }
    }; 
    ui_->onCloseFile = [this]() { 
        World3D::clear(); 
        this->session_->getNarrativeSystem().clear();
        this->session_->getDiscursiveSystemRepository().clear();
        this->session_->getRecommendationTrajectory().clear();
    }; 
    ui_->onExit = [this]() { this->running_ = false; };

    ui_->init(window_->getNativeWindow(), info);
    
    // Link Fourth Dimension System
    // Link Fourth Dimension System
    ui_->setupFourthDimension(&session_->getTrajectory(), session_->getLLMService());
    
    // Link Observational System
    ui_->setupObservational(session_.get());

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
