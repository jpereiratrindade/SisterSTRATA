#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.hpp>
#include <functional>
#include "application/dtos/UIData.hpp"

#include "ui/menus/MainMenu.hpp"
#include "ui/panels/AnalysisPanel.hpp"
#include "ui/panels/PatchAnalysisPanel.hpp"
#include "ui/panels/SettingsPanel.hpp"
#include "ui/panels/WelcomePanel.hpp"
#include "ui/panels/TerrainGeneratorPanel.hpp"
#include "ui/panels/TimelinePanel.hpp"
#include "ui/panels/VegetationDeclarationPanel.hpp"
#include "ui/panels/SoilSimPanel.hpp"
#include "ui/panels/HydrologyPanel.hpp"
#include "ui/panels/NarrativePanel.hpp"
#include "ui/panels/DiscursiveSystemPanel.hpp"
#include "ui/panels/RecommendationTrajectoryPanel.hpp"

namespace UI {

struct VulkanInitInfo {
    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::Queue queue;
    uint32_t queueFamily;
    vk::DescriptorPool descriptorPool;
    vk::RenderPass renderPass;
    uint32_t minImageCount;
    uint32_t imageCount;
};

class UserInterface {
public:
    void init(SDL_Window* window, const VulkanInitInfo& info);
    void setupFourthDimension(Core::Domain::FourthDimension::Trajectory* trajectory, Application::Ports::ILLMService* llmService);
    void setupObservational(Application::Session* session); // New
    void shutdown();

    // Callbacks
    std::function<void()> onLoadDemo;
    std::function<void(std::string)> onOpenFile; 
    std::function<void(std::string)> onSaveFile; // New
    std::function<void()> onCloseFile; 
    std::function<void()> onExit;
    
    void beginFrame();
    void draw(const Application::DTO::UIData& data);
    void render(vk::CommandBuffer cmd); // Final backend flush
    void endFrame(); 

    void processEvent(const SDL_Event* event);

    bool wantsToCaptureMouse() const;
    bool wantsToCaptureKeyboard() const;
    bool wantsTextInput() const; 

private:
    SDL_Window* window_ = nullptr;
    float dpiScale_ = 1.0f;

    // Components
    Menus::MainMenu mainMenu_;
    Panels::AnalysisPanel analysisPanel_;
    Panels::PatchAnalysisPanel patchAnalysisPanel_;
    Panels::SettingsPanel settingsPanel_;
    Panels::WelcomePanel welcomePanel_;
    Panels::VegetationDeclarationPanel vegetationDeclarationPanel_;
    Panels::SoilSimPanel soilSimPanel_;
    Panels::HydrologyPanel hydrologyPanel_;
    Panels::TerrainGeneratorPanel terrainGeneratorPanel_;
    Panels::TimelinePanel timelinePanel_; // New
    Panels::NarrativePanel narrativePanel_; // New
    Panels::DiscursiveSystemPanel discursiveSystemPanel_;
    Panels::RecommendationTrajectoryPanel recommendationTrajectoryPanel_;
};

} // namespace UI
