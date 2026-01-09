#include "ui/UI.hpp"
#include "world3d/World3D.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"
#include <SDL2/SDL_vulkan.h>
#include <algorithm>
#include <cmath>

namespace UI {

namespace {

float computeDpiScale(SDL_Window* window) {
    int w = 0, h = 0;
    SDL_GetWindowSize(window, &w, &h);

    int drawableW = 0, drawableH = 0;
    SDL_Vulkan_GetDrawableSize(window, &drawableW, &drawableH);

    if (w <= 0 || h <= 0) return 1.0f;

    const float scaleX = static_cast<float>(drawableW) / static_cast<float>(w);
    const float scaleY = static_cast<float>(drawableH) / static_cast<float>(h);

    if (!std::isfinite(scaleX) || !std::isfinite(scaleY) || scaleX <= 0.0f || scaleY <= 0.0f) return 1.0f;

    // Prefer a stable, uniform-ish scale (most platforms report uniform DPI scaling).
    return std::max(scaleX, scaleY);
}

void setupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Professional Dark Theme (Blender/Unreal inspired)
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(5, 3);
    style.CellPadding = ImVec2(4, 2);
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.TouchExtraPadding = ImVec2(0, 0);
    style.IndentSpacing = 21;
    style.ScrollbarSize = 14;
    style.GrabMinSize = 10;
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 0;
    style.TabBorderSize = 0;
    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 3.0f;
    style.LogSliderDeadzone = 4.0f;
    style.TabRounding = 4.0f;
}

void rebuildFontsForDpi(float dpiScale) {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig fontCfg;
    fontCfg.OversampleH = 3;
    fontCfg.OversampleV = 2;
    fontCfg.PixelSnapH = true;

    fontCfg.SizePixels = 13.0f * dpiScale;
    io.Fonts->AddFontDefault(&fontCfg);

    io.FontGlobalScale = (dpiScale > 0.0f) ? (1.0f / dpiScale) : 1.0f;
}

} // namespace

void UserInterface::init(SDL_Window* window, const VulkanInitInfo& info) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Docking disabled to match SisterAppPEC behavior.

    setupStyle();

    window_ = window;
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

    dpiScale_ = computeDpiScale(window_);
    rebuildFontsForDpi(dpiScale_);
    
    // Link Panels
    soilSimPanel_.setPatchAnalysisPanel(&patchAnalysisPanel_);
}

void UserInterface::setupFourthDimension(Core::Domain::FourthDimension::Trajectory* trajectory, Application::Ports::ILLMService* llmService) {
    timelinePanel_.setDependencies(trajectory, &vegetationDeclarationPanel_, llmService);
}

void UserInterface::setupObservational(Application::Session* session) {
    narrativePanel_.setSession(session);
    discursiveSystemPanel_.setSession(session);
    recommendationTrajectoryPanel_.setSession(session);
    timelinePanel_.setSession(session);
}

void UserInterface::shutdown() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    window_ = nullptr;
}

void UserInterface::processEvent(const SDL_Event* event) {
    ImGui_ImplSDL2_ProcessEvent(event);
}

void UserInterface::beginFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    if (window_) {
        // Fix HiDPI mismatch for Vulkan: SDL backend uses SDL_GL_GetDrawableSize(), which may be wrong without a GL context.
        int w = 0, h = 0;
        SDL_GetWindowSize(window_, &w, &h);
        int drawableW = 0, drawableH = 0;
        SDL_Vulkan_GetDrawableSize(window_, &drawableW, &drawableH);

        ImGuiIO& io = ImGui::GetIO();
        if (w > 0 && h > 0) {
            io.DisplayFramebufferScale = ImVec2(
                static_cast<float>(drawableW) / static_cast<float>(w),
                static_cast<float>(drawableH) / static_cast<float>(h)
            );
        }

        // Keep font scale fixed after init to avoid size jitter during resize.
    }

    ImGui::NewFrame();
}


void UserInterface::draw(const Application::DTO::UIData& data) {
    // 1. Update Callback Links (every frame or strictly on change? frame is safer for lambda captures if any)
    // Actually, MainMenu stores std::functions. Copying them every frame is cheap but slightly wasteful.
    // Better to linking them once in init? Or just copy here.
    mainMenu_.onLoadDemo = onLoadDemo;
    mainMenu_.onOpenFile = onOpenFile;
    mainMenu_.onSaveFile = onSaveFile; // Link
    mainMenu_.onCloseFile = onCloseFile;
    mainMenu_.onExit = onExit;

    // 2. Draw Main Menu
    mainMenu_.draw();

    // 3. Draw Panels (Logic controlled by MainMenu state)
    analysisPanel_.draw(&mainMenu_.showAnalysisReport);
    patchAnalysisPanel_.draw(&mainMenu_.showPatchAnalysis);
    settingsPanel_.draw(&mainMenu_.showSettings);
    hydrologyPanel_.draw(&mainMenu_.showHydrologyPanel);
    terrainGeneratorPanel_.draw(&mainMenu_.showTerrainGenerator); // New
    vegetationDeclarationPanel_.draw(&mainMenu_.showVegetation);
    welcomePanel_.draw(&mainMenu_.showWelcome, data);
    soilSimPanel_.drawScorpan(&mainMenu_.showScorpanWindow);
    soilSimPanel_.drawSiBCS(&mainMenu_.showSiBCSWindow);
    timelinePanel_.draw(&mainMenu_.showTimeline);
    narrativePanel_.draw(&mainMenu_.showNarrativePanel);
    discursiveSystemPanel_.draw(&mainMenu_.showDiscursivePanel);
    recommendationTrajectoryPanel_.draw(&mainMenu_.showRecommendationPanel);
}


void UserInterface::render(vk::CommandBuffer cmd) {
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


bool UserInterface::wantsToCaptureMouse() const {
    return ImGui::GetIO().WantCaptureMouse;
}

bool UserInterface::wantsToCaptureKeyboard() const {
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool UserInterface::wantsTextInput() const {
    return ImGui::GetIO().WantTextInput;
}

} // namespace UI
