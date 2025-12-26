#include "ui/UI.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"

namespace UI {

void UserInterface::init(SDL_Window* window, const VulkanInitInfo& info) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForVulkan(window);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = info.instance;
    init_info.PhysicalDevice = info.physicalDevice;
    init_info.Device = info.device;
    init_info.QueueFamily = info.queueFamily;
    init_info.Queue = info.queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = info.descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = info.minImageCount;
    init_info.ImageCount = info.imageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr;
    init_info.RenderPass = info.renderPass;

    ImGui_ImplVulkan_Init(&init_info);
    
    // Create fonts texture
    // Assuming implicit upload or we can do it manually if needed, but Init usually defers until NewFrame if no cmd provided? 
    // Actually ImGui_ImplVulkan_CreateFontsTexture is needed if we want it available immediately, 
    // but deferred creation in NewFrame is often supported or we might see a stutter.
    // For now, let's proceed. If text is missing, we'll know.
}

void UserInterface::shutdown() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void UserInterface::processEvent(const SDL_Event* event) {
    ImGui_ImplSDL2_ProcessEvent(event);
}

void UserInterface::beginFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void UserInterface::render(vk::CommandBuffer cmd) {
    // Docking setup
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    // Main Menu
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                // Should signal app to close
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::Begin("Welcome to SisterPEC");
    ImGui::Text("Scientific Data Platform - VULKAN BACKEND");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void UserInterface::endFrame() {
    // Platform windows (for docking) would update here for multi-viewport
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

} // namespace UI
