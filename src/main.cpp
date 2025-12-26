#include <iostream>
#include "infrastructure/window/Window.hpp"
#include "ui/UI.hpp"
#include "application/Session.hpp"
#include "application/dtos/UIData.hpp"
#include "world3d/World3D.hpp"

int main() {
    try {
        std::cout << "SisterPEC: Initializing..." << std::endl;

        // 1. Initialize Infrastructure (Window)
        // Note: Window is now initialized with SDL_WINDOW_VULKAN by default
        Infrastructure::Windowing::Window window("SisterPEC - Scientific Platform", 1280, 720);

        // 2. Initialize Vulkan (Team 3D) - MUST be before UI now
        World3D::init(window.getNativeWindow());
        World3D::loadDemoCloud();

        // 3. Initialize and bind UI (Team UI)
        UI::VulkanInitInfo info;
        info.instance = World3D::getInstance();
        info.physicalDevice = World3D::getPhysicalDevice();
        info.device = World3D::getDevice();
        info.queue = World3D::getGraphicsQueue();
        info.queueFamily = World3D::getGraphicsQueueFamilyIndex();
        info.descriptorPool = World3D::getDescriptorPool();
        info.renderPass = World3D::getRenderPass();
        info.minImageCount = World3D::getMinImageCount();
        info.imageCount = World3D::getImageCount();
        
        UI::UserInterface ui;
        
        bool running = true;
        ui.onLoadDemo = []() { World3D::loadDemoCloud(); };
        ui.onExit = [&running]() { running = false; };

        ui.init(window.getNativeWindow(), info);

        // 4. Initialize Application Session
        Application::Session session;

        // 5. Main Loop
        static Uint64 lastTime = SDL_GetPerformanceCounter();
        while (running && !window.shouldClose()) {
            Uint64 now = SDL_GetPerformanceCounter();
            float deltaTime = (float)(now - lastTime) / SDL_GetPerformanceFrequency();
            lastTime = now;
            // Poll Events
            // Poll Events
            window.pollEvents([&](const SDL_Event& event) {
                ui.processEvent(&event);
                
                // Only pass input to 3D world if UI is not capturing it
                bool uiMouse = ui.wantsToCaptureMouse();
                bool uiKeyboard = ui.wantsToCaptureKeyboard();

                if (!uiMouse && !uiKeyboard) {
                    World3D::processEvent(event);
                } else if (!uiKeyboard && (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)) {
                     // Allow keyboard if only mouse is captured (e.g. dragging a window but using WASD?)
                     // Usually if dragging a window we focus on UI.
                     // Safer rule: If UI wants mouse, don't rotate camera.
                     // Camera rotation is Mouse Motion.
                     World3D::processEvent(event);
                }
                
                // Refined Logic/Simplification:
                // Camera Controller handles both Mouse and Keyboard.
                // If ImGui wants Mouse, we shouldn't send Mouse Events.
                // If ImGui wants Keyboard, we shouldn't send Keyboard Events.
                
                // Let's filter by event type for robustness:
                bool isMouseEvent = (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEWHEEL);
                bool isKeyboardEvent = (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP);

                if (isMouseEvent && uiMouse) return;
                if (isKeyboardEvent && uiKeyboard) return;

                World3D::processEvent(event);
            });

            // Update
            // ... (Application logic would go here)

            // Render
            // Prepare 3D Frame (Acquire Image)
            World3D::beginFrame();
            
            // UI Logic Updates (NewFrame)
            ui.beginFrame();
            
            // Prepare Data Transfer Object (DTO) for UI
            Application::DTO::UIData uiData;
            uiData.framerate = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
            uiData.frameTimeMs = deltaTime * 1000.0f;
            uiData.startMessage = "System Ready";
            
            // Draw UI using only DTO data (Decoupled)
            ui.draw(uiData);
            
            // ui.render() is now deferred

            // Present 3D Frame + Overlay
            World3D::endFrame([&](vk::CommandBuffer cmd) {
                ui.render(cmd);
            });
            
            ui.endFrame(); // Clean up UI state (UpdatePlatformWindows etc)

            // window.swapBuffers(); // Handled by Vulkan Present
        }

        // 6. Cleanup
        World3D::getDevice().waitIdle(); // Ensure GPU is done before destroying
        ui.shutdown();
        World3D::shutdown();

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
