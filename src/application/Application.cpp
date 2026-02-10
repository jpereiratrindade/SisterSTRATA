#include "application/Application.hpp"
#include "world3d/World3D.hpp"
#include "application/dtos/UIData.hpp"
#include "infrastructure/llm/OllamaMockAdapter.hpp"
#include "infrastructure/llm/OllamaAdapter.hpp"
#include <iostream>
#include <SDL2/SDL.h>

namespace SisterSTRATA {

Application::Application() : Application(Config{}) {}

Application::Application(const Config& config) : config_(config) {
    init();
}

Application::~Application() {
    shutdown();
}

void Application::init() {
    std::cout << "[Application] Initializing..." << std::endl;

    // 1. Initialize Window
    window_ = std::make_unique<Infrastructure::Windowing::Window>("SisterSTRATA - Scientific Engine for Layered Landscapes", 1280, 720);

    if (!config_.useHybridMode) {
        // 2. Initialize World3D (Vulkan)
        World3D::init(window_->getNativeWindow());

        // 3. Initialize UI (Vulkan)
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
        ui_->init(window_->getNativeWindow(), info);
    } else {
        std::cout << "[Application] Initializing Hybrid View (CPU)..." << std::endl;
        hybridView_ = std::make_unique<::UI::Views::Hybrid2DView>();
        hybridView_->init(window_->getNativeWindow());
        
        // Initialize UI for Hybrid Mode
        ui_ = std::make_unique<::UI::UserInterface>();
        ui_->initHybrid(window_->getNativeWindow(), hybridView_->getRenderer());
    }

    // 4. Initialize Session
    session_ = std::make_unique<::Application::Session>();
    
    // ... LLM Service Setup (unchanged) ...
    auto realLLM = std::make_unique<::Infrastructure::LLM::OllamaAdapter>();
    if (realLLM->isAvailable()) {
        std::cout << "[Application] Ollama detected. Using " << realLLM->getModelName() << "." << std::endl;
        session_->setLLMService(std::move(realLLM));
    } else {
        std::cout << "[Application] Ollama not found. Falling back to Mock LLM." << std::endl;
        session_->setLLMService(std::make_unique<::Infrastructure::LLM::OllamaMockAdapter>());
    }

    // Wire Session to World View
    if (config_.useHybridMode) {
       session_->setWorldView(hybridView_.get());
    } else {
       session_->setWorldView(World3D::getWorldView());
    }
    
    if (ui_) {
        // Bind UI Callbacks (Shared between modes)
        ui_->onLoadDemo = [this]() { 
            if (!config_.useHybridMode) World3D::loadDemoCloud(); 
            else std::cout << "[Application] Load Demo ignored in CPU mode." << std::endl;
        };
        
        ui_->onOpenFile = [this](std::string path) { 
            if (this->session_) {
                if (!config_.useHybridMode) World3D::getDevice().waitIdle();
                this->session_->loadWorld(path);
            }
        };

        ui_->onSaveFile = [this](std::string path) { 
            bool baseSuccess = false;
            if (!config_.useHybridMode) {
                baseSuccess = World3D::saveFile(path);
            } else {
                baseSuccess = true; // Allow sidecar save even if 3D is missing
            }

            if (baseSuccess) {
                // Sidecar Save (Always allowed)
                try {
                    this->session_->getNarrativeSystem().serialize(path + ".json");
                } catch (...) {}
                try {
                    this->session_->getDiscursiveSystemRepository().serialize(path + ".discursive.json");
                } catch (...) {}
                try {
                    this->session_->getRecommendationTrajectory().serialize(path + ".recommendation.json");
                } catch (...) {}
                std::cout << "[Application] Observational data saved to sidecars." << std::endl;
            }
        }; 

        ui_->onCloseFile = [this]() { 
            if (!config_.useHybridMode) World3D::clear(); 
            this->session_->getNarrativeSystem().clear();
            this->session_->getDiscursiveSystemRepository().clear();
            this->session_->getRecommendationTrajectory().clear();
        }; 
        
        // Project Management Bindings (Crucial for SGS)
        ui_->onNewProject = [this](std::string path) {
            std::cout << "[Application] Creating New Project at: " << path << std::endl;
            if (!config_.useHybridMode) World3D::getDevice().waitIdle();
            this->session_->setProjectRoot(path);
        };

        ui_->onOpenProject = [this](std::string path) {
            std::cout << "[Application] Opening Project from: " << path << std::endl;
            if (!config_.useHybridMode) World3D::getDevice().waitIdle();
            this->session_->setProjectRoot(path);
        };

        ui_->onExit = [this]() { this->running_ = false; };
        
        ui_->onImportIW = [this](std::string path) {
            if (this->session_) {
                this->session_->ingestFromIWDirectory(path);
            }
        };

        // Link Systems (both modes)
        ui_->setupFourthDimension(&session_->getTrajectory(), session_->getLLMService());
        ui_->setupObservational(session_.get());
    }

    std::cout << "[Application] Initialization Complete." << std::endl;
}

void Application::shutdown() {
    if (window_) {
        if (ui_) ui_->shutdown();

        if (!config_.useHybridMode) {
            // Ensure GPU is idle before destroying resources
            World3D::getDevice().waitIdle();
            World3D::shutdown();
        }
        
        ui_.reset();
        hybridView_.reset();
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

        if (config_.useHybridMode) {
            // Hybrid Render
            if (hybridView_) hybridView_->render();
            
            // Draw UI over Hybrid
            if (ui_) {
                ui_->beginFrame();
                ::Application::DTO::UIData uiData;
                uiData.framerate = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
                uiData.frameTimeMs = deltaTime * 1000.0f;
                uiData.startMessage = "System Ready (CPU Mode)";
                if (hybridView_) {
                     uiData.startMessage += " [View: " + hybridView_->getViewName() + " (TAB)]";
                }
                ui_->draw(uiData);
                ui_->renderHybrid();
                ui_->endFrame();
            }

            // Final Presentation for Hybrid Mode
            if (hybridView_) {
                SDL_RenderPresent(hybridView_->getRenderer());
            }
        } else {
            // Vulkan Render
            // 1. Prepare 3D Frame
            World3D::beginFrame();
            
            // 2. Update UI Logic
            if (ui_) {
                ui_->beginFrame();
                ::Application::DTO::UIData uiData;
                uiData.framerate = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
                uiData.frameTimeMs = deltaTime * 1000.0f;
                uiData.startMessage = "System Ready";
                ui_->draw(uiData);
            }
            
            // 3. Present (Render 3D + UI Overlay)
            World3D::endFrame([this](vk::CommandBuffer cmd) {
                if (this->ui_) this->ui_->render(cmd);
            });
            
            if (ui_) ui_->endFrame();
        }
    }
}

void Application::processEvents() {
    window_->pollEvents([this](const SDL_Event& event) {
        // 1. Pass to UI first
        if (ui_) {
            ui_->processEvent(&event);
        }
        
        // 2. Determine if 3D World should receive input
        bool uiMouse = ui_ ? ui_->wantsToCaptureMouse() : false;
        bool uiKeyboard = ui_ ? ui_->wantsToCaptureKeyboard() : false;

        // 3. Input Routing Logic
        bool isMouseEvent = (event.type == SDL_MOUSEMOTION || 
                           event.type == SDL_MOUSEBUTTONDOWN || 
                           event.type == SDL_MOUSEBUTTONUP || 
                           event.type == SDL_MOUSEWHEEL);
                           
        bool isKeyboardEvent = (event.type == SDL_KEYDOWN || 
                              event.type == SDL_KEYUP);

        if (!config_.useHybridMode) {
             if (isMouseEvent && uiMouse) return;
             // Allow keyboard shortcuts (WASD) unless text input is active
             if (isKeyboardEvent && ui_ && ui_->wantsTextInput()) return;
             
             // 4. Pass to World3D
             World3D::processEvent(event);
        } else {
            // Hybrid Input
            if (hybridView_) hybridView_->handleEvent(event);

            if (ui_) {
                 // Forward to UI first
                 ui_->processEvent(&event);
                 if (ui_->wantsToCaptureMouse() || ui_->wantsToCaptureKeyboard()) return;
            }

            if (event.type == SDL_KEYDOWN) {
                 if (event.key.keysym.sym == SDLK_ESCAPE) this->running_ = false;
                 // Demo load trigger for Hybrid?
                 if (event.key.keysym.sym == SDLK_l) {
                     // Trigger simple load
                     session_->loadWorld("demo.obj"); // Dummy
                 }
            }
        }
    });
}


} // namespace SisterSTRATA
