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
    static bool showWelcome = true;
    static bool showSettings = false;
    static bool showAnalysisReport = false;
    static bool showTerrainGen = false;
    static bool showOpenDialog = false;
    static char filePathBuf[256] = "assets/data/sample.csv";

    // Main Menu
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open File...", "Ctrl+O")) {
                showOpenDialog = true;
            }
            if (ImGui::MenuItem("Load Demo Cloud")) {
                if (onLoadDemo) onLoadDemo();
            }
            if (ImGui::MenuItem("Close File")) {
                if (onCloseFile) onCloseFile();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                if (onExit) onExit();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Welcome Panel", nullptr, &showWelcome);
            // ImGui::MenuItem("ImGui Demo", nullptr, &showDemo);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Settings")) {
                showSettings = true;
            }
            if (ImGui::MenuItem("Analyze Slope")) {
                World3D::applySlopeAnalysis();
                showAnalysisReport = true;
            }
            if (ImGui::MenuItem("Generate Pattern")) {
                showTerrainGen = true;
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }

    if (showAnalysisReport) {
        if (ImGui::Begin("Analysis Report", &showAnalysisReport)) {
            auto stats = World3D::getSlopeAnalysisStats();
            
            if (stats.total > 0) {
                ImGui::Text("Total Vertices: %d", stats.total);
                ImGui::Separator();
                
                auto drawRow = [&](const char* label, int count, ImVec4 color) {
                    float pct = (float)count / (float)stats.total * 100.0f;
                    ImGui::TextColored(color, "%s: %d (%.1f%%)", label, count, pct);
                };

                drawRow("Flat (0-5 deg)", stats.countFlat, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
                drawRow("Gentle (5-20 deg)", stats.countGentle, ImVec4(0.8f, 0.8f, 0.2f, 1.0f));
                drawRow("Moderate (20-45 deg)", stats.countModerate, ImVec4(0.9f, 0.5f, 0.0f, 1.0f));
                drawRow("Steep (>45 deg)", stats.countSteep, ImVec4(0.9f, 0.1f, 0.1f, 1.0f));
                
                ImGui::Separator();
                
                static char reportFilename[128] = "slope_report.txt";
                ImGui::InputText("Filename", reportFilename, IM_ARRAYSIZE(reportFilename));
                
                if (ImGui::Button("Save Report")) {
                    if (World3D::saveReport(reportFilename)) {
                        ImGui::OpenPopup("Saved");
                    }
                }
                
                if (ImGui::BeginPopupModal("Saved", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Report saved successfully!");
                    if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                    ImGui::EndPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Close")) {
                    showAnalysisReport = false;
                }
            } else {
                ImGui::Text("No data analyzed.");
            }
        }
        ImGui::End();
    }
    
    // Terrain Generator Dialog
    if (showTerrainGen) {
        if (ImGui::Begin("Generate Terrain", &showTerrainGen)) {
            static char genFilename[256] = "assets/data/generated_terrain.obj";
            static int genWidth = 100;
            static int genHeight = 100;
            static float genSpacing = 1.0f;
            static int genType = 0;
            const char* typeItems[] = { "Hills", "Mountains", "Flat", "Canyon" };

            ImGui::InputText("Output Filename", genFilename, IM_ARRAYSIZE(genFilename));
            ImGui::InputInt("Width", &genWidth);
            ImGui::InputInt("Height", &genHeight);
            ImGui::InputFloat("Spacing (m)", &genSpacing);
            ImGui::Combo("Pattern", &genType, typeItems, IM_ARRAYSIZE(typeItems));
            
            // Validate inputs
            if (genWidth < 10) genWidth = 10;
            if (genHeight < 10) genHeight = 10;
            if (genSpacing < 0.1f) genSpacing = 0.1f;
            
            ImGui::Separator();

            bool isGen = World3D::isTerrainGenerating();
            if (isGen) {
                ImGui::Text("Generating... Please wait.");
            } else {
                if (ImGui::Button("Generate & Load")) {
                    if (World3D::generateTerrain(genFilename, genWidth, genHeight, genSpacing, genType, true)) {
                        // Load handled automatically by Engine on success
                    }
                }
                
                ImGui::SameLine();
                 
                 if (ImGui::Button("Generate Only")) {
                    if (World3D::generateTerrain(genFilename, genWidth, genHeight, genSpacing, genType, false)) {
                         ImGui::OpenPopup("GenSuccess");
                    }
                 }
            }

             if (ImGui::BeginPopupModal("GenSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                 ImGui::Text("Terrain Generation Started!");
                 if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); showTerrainGen = false; }
                 ImGui::EndPopup();
             }
        }
        ImGui::End();
    }
    
    if (showSettings) {
        if (ImGui::Begin("Settings", &showSettings)) {
            if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
                 glm::vec3 lightDir = World3D::getLightDirection();
                 if (ImGui::SliderFloat3("Light Direction", &lightDir.x, -5.0f, 5.0f)) {
                     World3D::setLightDirection(lightDir.x, lightDir.y, lightDir.z);
                 }
    
                 glm::vec3 lightColor = World3D::getLightColor();
                 if (ImGui::ColorEdit3("Light Color", &lightColor.r)) {
                     World3D::setLightColor(lightColor.r, lightColor.g, lightColor.b);
                 }
    
                 float ambient = World3D::getAmbientStrength();
                 if (ImGui::SliderFloat("Ambient Strength", &ambient, 0.0f, 1.0f)) {
                     World3D::setAmbientStrength(ambient);
                 }
            
                static float moveSpeed = 2.5f;
                if (ImGui::SliderFloat("Camera Speed", &moveSpeed, 0.1f, 100.0f)) {
                    World3D::setCameraSpeed(moveSpeed);
                }
            }
        }
        ImGui::End();
    }

    if (showWelcome) {
        ImGuiIO& io = ImGui::GetIO();
        // Position on top-right, similar to Inspector in reference
        const float panelMargin = 10.0f;
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 320.0f - panelMargin, panelMargin + 18.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(320.0f, 200.0f), ImGuiCond_FirstUseEver); // Allow resize
        
        if (ImGui::Begin("Welcome to SisterPEC", &showWelcome)) {
            ImGui::Text("Scientific Data Platform - VULKAN BACKEND");
            ImGui::Separator();
            
            // Use DTO Data
            ImGui::Text("FPS: %.1f", data.framerate);
            ImGui::Text("Frame Time: %.3f ms", data.frameTimeMs);
            
            if (!data.startMessage.empty()) {
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", data.startMessage.c_str());
            }

            if (ImGui::Button("Reset Camera")) {
                 // TODO: Reset Camera logic
            }

            ImGui::Separator();
        
            ImGui::Separator();
            // Lighting controls moved to Settings
            ImGui::TextDisabled("Lighting controls moved to Tools > Settings");

        }
        ImGui::End();
    }
    
    // if (showDemo) {
    //     ImGui::ShowDemoWindow(&showDemo);
    // }

    // File Open Modal
    if (showOpenDialog) {
        ImGui::OpenPopup("Open File");
    }

    if (ImGui::BeginPopupModal("Open File", &showOpenDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter file path:");
        ImGui::InputText("Path", filePathBuf, IM_ARRAYSIZE(filePathBuf));
        
        ImGui::Separator();
        
        if (ImGui::Button("Load", ImVec2(120, 0))) {
            if (onOpenFile) onOpenFile(std::string(filePathBuf));
            showOpenDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            showOpenDialog = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
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
