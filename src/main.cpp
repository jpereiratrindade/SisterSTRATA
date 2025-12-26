#include <iostream>
#include "infrastructure/window/Window.hpp"
#include "ui/UI.hpp"
#include "application/Session.hpp"
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
        ui.init(window.getNativeWindow(), info);

        // 4. Initialize Application Session
        Application::Session session;

        // 5. Main Loop
        bool running = true;
        while (running && !window.shouldClose()) {
            // Poll Events
            window.pollEvents([&](const SDL_Event& event) {
                ui.processEvent(&event);
                World3D::processEvent(event);
            });

            // Update
            // ... (Application logic would go here)

            // Render
            // Prepare 3D Frame (Acquire Image)
            World3D::beginFrame();
            
            // Render UI into the current command buffer
            ui.beginFrame();
            ui.render(World3D::getCurrentCommandBuffer()); 
            ui.endFrame();

            // Present 3D Frame (Submit and Present)
            World3D::endFrame();

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
