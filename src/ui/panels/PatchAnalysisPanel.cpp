#include "PatchAnalysisPanel.hpp"
#include "imgui.h"
#include "world3d/World3D.hpp"
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <glm/glm.hpp>
#include "core/value_objects/Vector3.hpp"
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <cstdio>
#include <cstring>

namespace {

ImU32 ColorForLabel(uint32_t label, int paletteMode) {
    if (label == 0u) {
        return IM_COL32(30, 30, 30, 255);
    }

    auto hashColor = [](uint32_t v) -> ImU32 {
        uint32_t h = v * 2654435761u;
        h ^= (h >> 13);
        h *= 0x9E3779B1u;
        uint8_t r = static_cast<uint8_t>((h >> 16) & 0xFF);
        uint8_t g = static_cast<uint8_t>((h >> 8) & 0xFF);
        uint8_t b = static_cast<uint8_t>(h & 0xFF);
        if (r == 0 && g == 0 && b == 0) b = 255;
        return IM_COL32(r, g, b, 255);
    };

    static const ImU32 vivid[] = {
        IM_COL32(231, 76, 60, 255),
        IM_COL32(46, 204, 113, 255),
        IM_COL32(52, 152, 219, 255),
        IM_COL32(241, 196, 15, 255),
        IM_COL32(155, 89, 182, 255),
        IM_COL32(26, 188, 156, 255),
        IM_COL32(230, 126, 34, 255),
        IM_COL32(149, 165, 166, 255),
    };

    static const ImU32 pastel[] = {
        IM_COL32(244, 199, 195, 255),
        IM_COL32(199, 234, 212, 255),
        IM_COL32(197, 222, 243, 255),
        IM_COL32(249, 236, 191, 255),
        IM_COL32(220, 200, 235, 255),
        IM_COL32(194, 234, 229, 255),
        IM_COL32(248, 218, 193, 255),
        IM_COL32(220, 226, 228, 255),
    };

    const size_t idx = static_cast<size_t>((label - 1u) % 8u);
    if (paletteMode == 1) {
        return vivid[idx];
    }
    if (paletteMode == 2) {
        return pastel[idx];
    }
    return hashColor(label);
}

glm::vec3 ColorForLabel3D(uint32_t label, int paletteMode) {
    if (label == 0u) {
        return glm::vec3(0.12f, 0.12f, 0.12f);
    }
    auto toVec3 = [](ImU32 color) {
        float r = static_cast<float>((color >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f;
        float g = static_cast<float>((color >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f;
        float b = static_cast<float>((color >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f;
        return glm::vec3(r, g, b);
    };
    return toVec3(ColorForLabel(label, paletteMode));
}

bool LoadLegendFile(const std::string& path,
                    std::vector<UI::Panels::LegendEntry>& entries,
                    std::string& error) {
    std::ifstream file(path);
    if (!file.is_open()) {
        error = "Unable to open legend file.";
        return false;
    }

    entries.clear();
    std::string line;
    int lineNo = 0;
    while (std::getline(file, line)) {
        ++lineNo;
        auto commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> parts;
        while (std::getline(ss, token, ',')) {
            if (!token.empty() && token.back() == '\r') token.pop_back();
            parts.push_back(token);
        }
        if (parts.size() < 5) {
            error = "Legend requires: id,name,r,g,b (line " + std::to_string(lineNo) + ")";
            continue; 
        }

        // Skip header if detected
        if (lineNo == 1 && (parts[0] == "id" || parts[0] == "ID")) {
            continue;
        }

        try {
            int id = std::stoi(parts[0]);
            std::string name = parts[1];
            double r = std::stod(parts[2]);
            double g = std::stod(parts[3]);
            double b = std::stod(parts[4]);
            
            if (r > 1.0 || g > 1.0 || b > 1.0) {
                r /= 255.0;
                g /= 255.0;
                b /= 255.0;
            }

            UI::Panels::LegendEntry entry;
            entry.id = id;
            entry.name = name;
            entry.color = ImVec4(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), 1.0f);
            entries.push_back(entry);
        } catch (const std::exception& e) {
             // Log or ignore malformed lines
             std::fprintf(stderr, "[PatchAnalysisPanel] Warning: Failed to parse legend line %d: %s\n", lineNo, e.what());
             continue;
        }
    }

    if (entries.empty()) {
        error = "Legend file is empty.";
        return false;
    }
    return true;
}

} // namespace

namespace UI::Panels {

void PatchAnalysisPanel::draw(bool* open) {
    if (!open || !(*open)) return;

    if (!ImGui::Begin("Patch Analysis (CSV)", open)) {
        ImGui::End();
        return;
    }

    ImGui::TextUnformatted("Input Grid (CSV Matrix)");
    ImGui::InputText("CSV Path", state_.inputPath.data(), state_.inputPath.size());
    ImGui::SameLine();
    if (ImGui::Button("Browse...##Input")) {
        state_.showFileSelector = true;
        state_.fileSelector.Open(state_.inputPath.data());
    }

    // Handle Popup
    std::string selectedPath;
    if (state_.fileSelector.draw(&state_.showFileSelector, selectedPath, ".csv")) {
        if (selectedPath.size() < state_.inputPath.size()) {
            std::strncpy(state_.inputPath.data(), selectedPath.c_str(), state_.inputPath.size() - 1);
            state_.inputPath[state_.inputPath.size() - 1] = '\0';
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Cell Metrics");
    ImGui::InputDouble("Cell Width", &state_.cellWidth, 0.1, 1.0, "%.6f");
    ImGui::InputDouble("Cell Height", &state_.cellHeight, 0.1, 1.0, "%.6f");
    if (state_.cellWidth <= 0.0) state_.cellWidth = 1.0;
    if (state_.cellHeight <= 0.0) state_.cellHeight = 1.0;

    ImGui::Separator();
    ImGui::TextUnformatted("Segmentation");
    bool thresholdMode = !state_.byClass;
    if (ImGui::RadioButton("Single threshold", thresholdMode)) {
        state_.byClass = false;
    }
    ImGui::SameLine();
    bool classMode = state_.byClass;
    if (ImGui::RadioButton("By class (ignore threshold)", classMode)) {
        state_.byClass = true;
    }
    if (!state_.byClass) {
        ImGui::InputDouble("Threshold", &state_.threshold, 0.1, 1.0, "%.6f");
    } else {
        ImGui::TextDisabled("By class ignores zeros. Use 0 as background.");
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Outputs");
    ImGui::Checkbox("Summary only (CSV)", &state_.summaryOnly);
    ImGui::Checkbox("Write CSV metrics", &state_.exportCsv);
    if (state_.exportCsv) {
        ImGui::InputText("Metrics CSV", state_.outputCsvPath.data(), state_.outputCsvPath.size());
    }
    ImGui::Checkbox("Write label grid", &state_.exportLabels);
    if (state_.exportLabels) {
        ImGui::InputText("Labels CSV", state_.outputLabelsPath.data(), state_.outputLabelsPath.size());
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Preview Styling");
    ImGui::Checkbox("Show preview", &state_.showPreview);
    ImGui::SliderFloat("Cell size", &state_.previewCellSize, 2.0f, 20.0f, "%.1f");
    const char* palettes[] = {"Hash", "Vivid", "Pastel"};
    ImGui::Combo("Palette", &state_.paletteMode, palettes, IM_ARRAYSIZE(palettes));
    const char* shapes[] = {"Square", "Rounded", "Circle"};
    ImGui::Combo("Shape", &state_.shapeMode, shapes, IM_ARRAYSIZE(shapes));

    ImGui::Separator();
    ImGui::TextUnformatted("Legend");
    ImGui::InputText("Legend CSV", state_.legendPath.data(), state_.legendPath.size());
    ImGui::Checkbox("Use legend colors", &state_.useLegendColors);
    ImGui::SameLine();
    if (ImGui::Button("Load Legend")) {
        state_.error.clear();
        if (LoadLegendFile(state_.legendPath.data(), state_.legendEntries, state_.error)) {
            state_.legendLoaded = true;
            state_.status = "Legend loaded.";
        } else {
            state_.legendLoaded = false;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Use SiBCS Legend")) {
        std::snprintf(state_.legendPath.data(), state_.legendPath.size(), "%s", "assets/data/sibcs_legend.csv");
        state_.error.clear();
        if (LoadLegendFile(state_.legendPath.data(), state_.legendEntries, state_.error)) {
            state_.legendLoaded = true;
            state_.byClass = true;
            state_.useLegendColors = true;
            state_.status = "SiBCS legend loaded.";
        } else {
            state_.legendLoaded = false;
        }
    }

    if (state_.legendLoaded && !state_.legendEntries.empty()) {
        ImGui::BeginChild("LegendList", ImVec2(0.0f, 120.0f), true);
        for (const auto& entry : state_.legendEntries) {
            ImGui::PushID(entry.id);
            ImGui::ColorButton("legendColor", entry.color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(12, 12));
            ImGui::SameLine();
            ImGui::Text("%d - %s", entry.id, entry.name.c_str());
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    ImGui::Separator();
    ImGui::TextUnformatted("3D View");
    ImGui::Checkbox("Use patch labels for color", &state_.useLabelColors);
    ImGui::Checkbox("Use value as height", &state_.useValueAsHeight);
    ImGui::SliderFloat("Height scale", &state_.heightScale, 0.1f, 10.0f, "%.2f");
    if (ImGui::SliderFloat("Point size", &state_.pointSize, 1.0f, 20.0f, "%.1f")) {
        World3D::setPointSize(state_.pointSize);
    }
    
    ImGui::Separator();
    ImGui::Text("Alignment Correction");
    ImGui::Checkbox("Enable Manual Offset", &state_.useManualOffset);
    if (state_.useManualOffset) {
        ImGui::DragFloat3("XYZ Offset", &state_.manualOffset.x, 0.5f, -1000.0f, 1000.0f);
    }
    if (ImGui::Button("Load grid to 3D")) {
        state_.error.clear();
        try {
            auto grid = Core::Domain::SpatialPattern::LoadGridCsv(state_.inputPath.data());
            
            // Try to load elevation companion file immediately
            if (grid.elevation.empty()) {
                std::string input(state_.inputPath.data());
                std::string target = "soil_raster_";
                auto pos = input.find(target);
                if (pos != std::string::npos) {
                    std::string base = input.substr(0, pos) + "soil_elevation_" + input.substr(pos + target.length());
                    try {
                        auto elevGrid = Core::Domain::SpatialPattern::LoadGridCsv(base);
                        if (elevGrid.width == grid.width && elevGrid.height == grid.height) {
                            grid.elevation = elevGrid.values;
                        }
                    } catch(...) {}
                }
            }

            std::vector<Core::ValueObjects::Vector3> points;
            std::vector<glm::vec3> colors;
            points.reserve(grid.values.size());
            colors.reserve(grid.values.size());

            const bool haveLabels = !state_.lastLabels.labels.empty() &&
                                    state_.lastLabels.width == grid.width &&
                                    state_.lastLabels.height == grid.height;
            
            std::unordered_map<int, ImVec4> legendById;
            if (state_.legendLoaded && state_.useLegendColors) {
                for (const auto& entry : state_.legendEntries) {
                    legendById[entry.id] = entry.color;
                }
            }

            // Calculate Origin / Offset
            double startX = 0.0;
            double startY = 0.0;
            
            // Check if origin metadata was loaded
            bool hasOrigin = (std::abs(grid.originX) > 1e-6 || std::abs(grid.originY) > 1e-6);
            
            // Also check if elevation is present - if so, we strongly prefer absolute positioning
            // assuming the elevation grid matches the raster grid spatially.
            // If origin is missing but elevation exists, we might default to 0,0 or Center?
            // Actually, if elevation exists, it implies we are overlaying on specific terrain.
            // Using 0,0 as start (absolute) is safer than centering if origin is missing but elevation present?
            // But if origin is -512, hasOrigin is true.
            
            if (hasOrigin) {
                startX = grid.originX;
                startY = grid.originY;
            } else if (!grid.elevation.empty()) {
            } else {
                 // Classic Centering
                 startX = -(grid.width - 1) * 0.5 * state_.cellWidth;
                 startY = -(grid.height - 1) * 0.5 * state_.cellHeight;
            }
            
            // Apply Manual Offset
            if (state_.useManualOffset) {
                startX += state_.manualOffset.x;
                startY += state_.manualOffset.y;
            }

            for (int y = 0; y < grid.height; ++y) {
                for (int x = 0; x < grid.width; ++x) {
                    size_t idx = static_cast<size_t>(y * grid.width + x);
                    double val = grid.values[idx];
                    if (grid.hasNoData && std::isnan(val)) continue;
                    if (state_.byClass) {
                        if (static_cast<int32_t>(val) == 0) continue;
                    } else if (!(val > state_.threshold)) {
                        continue;
                    }

                    double z = 0.0;
                    if (state_.useValueAsHeight) {
                        z = val * state_.heightScale;
                    } else {
                        if (!grid.elevation.empty()) {
                            z = grid.elevation[idx] + 0.1; 
                        } else {
                             z = val * state_.heightScale; 
                        }
                    }
                    
                    if (state_.useManualOffset) {
                        z += state_.manualOffset.z;
                    }
                    
                    points.push_back({startX + x * state_.cellWidth, startY + y * state_.cellHeight, z});

                    uint32_t label = 1u;
                    if (state_.useLabelColors && haveLabels) {
                        label = state_.lastLabels.labels[idx];
                    } else if (state_.byClass) {
                        label = static_cast<uint32_t>(std::max(0, static_cast<int>(val)));
                    }
                    if (!legendById.empty()) {
                        auto it = legendById.find(static_cast<int>(label));
                        if (it != legendById.end()) {
                            colors.push_back(glm::vec3(it->second.x, it->second.y, it->second.z));
                        } else {
                            colors.push_back(ColorForLabel3D(label, state_.paletteMode));
                        }
                    } else {
                        colors.push_back(ColorForLabel3D(label, state_.paletteMode));
                    }
                }
            }

            if (points.empty()) {
                throw std::runtime_error("No points to load after filtering.");
            }
            World3D::setPointSize(state_.pointSize);
            World3D::loadPointCloud(points, colors, "grid-points");
            state_.status = "Grid sent to 3D view.";
        } catch (const std::exception& e) {
            state_.error = e.what();
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("3D Snapshot");
    ImGui::InputText("Snapshot PNG", state_.snapshotPath.data(), state_.snapshotPath.size());
    if (ImGui::Button("Save 3D Snapshot")) {
        state_.error.clear();
        if (!World3D::requestScreenshot(state_.snapshotPath.data())) {
            state_.error = "Unable to request screenshot.";
        } else {
            state_.status = "Screenshot requested.";
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Run Patch Analysis")) {
        state_.error.clear();
        state_.status.clear();
        state_.summary = Core::Domain::SpatialPattern::SummaryMetrics{};
        state_.lastRunSuccess = false;
        state_.lastLabels = Core::Domain::SpatialPattern::LabelImage{};
        try {
            auto grid = Core::Domain::SpatialPattern::LoadGridCsv(state_.inputPath.data());
            grid.cellWidth = state_.cellWidth;
            grid.cellHeight = state_.cellHeight;

            Core::Domain::SpatialPattern::AnalysisConfig cfg;
            cfg.threshold = state_.threshold;
            cfg.byClass = state_.byClass;
            cfg.keepLabels = state_.exportLabels || state_.showPreview;

            auto result = Core::Domain::SpatialPattern::AnalyzeGrid(grid, cfg);
            state_.summary = result.summary;
            if (cfg.keepLabels) {
                state_.lastLabels = result.labelImage;
            }

            if (state_.exportCsv) {
                if (state_.outputCsvPath[0] == '\0') {
                    throw std::runtime_error("Output CSV path is empty.");
                }
                Core::Domain::SpatialPattern::WriteCsv(state_.outputCsvPath.data(), result.patches, result.summary, state_.summaryOnly);
            }
            if (state_.exportLabels) {
                if (state_.outputLabelsPath[0] == '\0') {
                    throw std::runtime_error("Label CSV path is empty.");
                }
                Core::Domain::SpatialPattern::WriteLabelCsv(state_.outputLabelsPath.data(), result.labelImage);
            }

            std::ostringstream oss;
            oss << "Completed. Patches: " << result.summary.patchCount
                << " | S1=" << std::fixed << std::setprecision(6) << result.summary.s1
                << " | S2=" << result.summary.s2;
            state_.status = oss.str();
            state_.lastRunSuccess = true;
        } catch (const std::exception& e) {
            state_.error = e.what();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Results")) {
        clearResults();
    }

    if (state_.showPreview) {
        ImGui::Separator();
        ImGui::TextUnformatted("Preview");
        if (state_.lastLabels.labels.empty()) {
            ImGui::TextDisabled("Run analysis to generate labels for preview.");
        } else {
            std::unordered_map<int, ImU32> legendById;
            if (state_.legendLoaded && state_.useLegendColors) {
                for (const auto& entry : state_.legendEntries) {
                    legendById[entry.id] = ImGui::ColorConvertFloat4ToU32(entry.color);
                }
            }
            const float cell = state_.previewCellSize;
            const float width = state_.lastLabels.width * cell;
            const float height = state_.lastLabels.height * cell;
            ImGui::BeginChild("PatchPreview", ImVec2(0.0f, 260.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
            ImVec2 start = ImGui::GetCursorScreenPos();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            for (int y = 0; y < state_.lastLabels.height; ++y) {
                for (int x = 0; x < state_.lastLabels.width; ++x) {
                    size_t idx = static_cast<size_t>(y * state_.lastLabels.width + x);
                    uint32_t label = state_.lastLabels.labels[idx];
                    ImU32 col = ColorForLabel(label, state_.paletteMode);
                    auto it = legendById.find(static_cast<int>(label));
                    if (it != legendById.end()) {
                        col = it->second;
                    }
                    ImVec2 min = ImVec2(start.x + x * cell, start.y + y * cell);
                    ImVec2 max = ImVec2(min.x + cell, min.y + cell);
                    if (state_.shapeMode == 2) {
                        ImVec2 center = ImVec2(min.x + cell * 0.5f, min.y + cell * 0.5f);
                        drawList->AddCircleFilled(center, cell * 0.45f, col);
                    } else {
                        float rounding = (state_.shapeMode == 1) ? (cell * 0.25f) : 0.0f;
                        drawList->AddRectFilled(min, max, col, rounding);
                    }
                }
            }
            ImGui::Dummy(ImVec2(width, height));
            ImGui::EndChild();
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Patch Shape Metrics");
    ImGui::BulletText("PAR = perimeter / area");
    ImGui::BulletText("Shape Index = (0.282 * P) / sqrt(A)");
    ImGui::BulletText("RCC = 2 * sqrt(A/pi) / max axis");
    ImGui::BulletText("S1 = 1 / (N * sum(L/S))");
    ImGui::BulletText("S2 = 1 / (N * sum(P/(4*sqrt(A))))");

    if (!state_.status.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.3f, 1.0f), "%s", state_.status.c_str());
        if (state_.summary.patchCount > 0) {
            ImGui::Text("Patches: %d | S1=%.6f | S2=%.6f",
                        state_.summary.patchCount,
                        state_.summary.s1,
                        state_.summary.s2);
            ImGui::Text("Means -> PAR=%.6f | SI=%.6f | RCC=%.6f",
                        state_.summary.meanPar,
                        state_.summary.meanShapeIndex,
                        state_.summary.meanRcc);
        }
    }

    if (!state_.error.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.2f, 1.0f), "Error: %s", state_.error.c_str());
    }

    ImGui::End();
}

void PatchAnalysisPanel::clearResults() {
    state_.summary = Core::Domain::SpatialPattern::SummaryMetrics{};
    state_.lastRunSuccess = false;
    state_.lastLabels = Core::Domain::SpatialPattern::LabelImage{};
    state_.legendEntries.clear();
    state_.legendLoaded = false;
    state_.status.clear();
    state_.error.clear();
    
    // Fix: Also clear the 3D visualization
    World3D::clear();
}

void PatchAnalysisPanel::SetInputPath(const std::string& path) {
    std::snprintf(state_.inputPath.data(), state_.inputPath.size(), "%s", path.c_str());
    state_.status = "Input path set externally. Ready to run.";
}

void PatchAnalysisPanel::SetLegendPath(const std::string& path) {
    std::snprintf(state_.legendPath.data(), state_.legendPath.size(), "%s", path.c_str());
}

} // namespace UI::Panels
