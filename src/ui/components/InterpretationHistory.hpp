#pragma once

#include "src/application/dtos/cognitive/InterpretationSnapshotDTO.hpp"
#include "ui/components/FileSelector.hpp"
#include <imgui.h>
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace UI::Components {

/**
 * @brief Reusable component to list, select, view and export Interpretation Snapshots.
 */
class InterpretationHistory {
public:
    static void Draw(const std::vector<Application::DTO::Cognitive::InterpretationSnapshotDTO>& snapshots) {
        static int selectedIndex = -1;
        static bool showDetailModal = false;
        static std::set<std::string> selectedSnapshotIds;

        static FileSelector exportSelector{"Export LLM Evaluations (.md)"};
        static bool showExportDialog = false;
        static std::string exportPathResult;
        static std::string exportMarkdown;
        static std::string exportError;
        static std::string lastExportPath = "assets/data/";

        auto clearStaleSelections = [&]() {
            std::set<std::string> validIds;
            for (const auto& snap : snapshots) {
                validIds.insert(snap.snapshotId);
            }
            for (auto it = selectedSnapshotIds.begin(); it != selectedSnapshotIds.end();) {
                if (!validIds.contains(*it)) it = selectedSnapshotIds.erase(it);
                else ++it;
            }
        };

        clearStaleSelections();

        if (snapshots.empty()) {
            selectedSnapshotIds.clear();
            ImGui::TextDisabled("No interpretation history available.");
            return;
        }

        auto selectedSnapshots = [&]() {
            std::vector<Application::DTO::Cognitive::InterpretationSnapshotDTO> out;
            for (const auto& snap : snapshots) {
                if (selectedSnapshotIds.contains(snap.snapshotId)) {
                    out.push_back(snap);
                }
            }
            return out;
        };

        if (ImGui::Button("Select All##History")) {
            selectedSnapshotIds.clear();
            for (const auto& snap : snapshots) selectedSnapshotIds.insert(snap.snapshotId);
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear Selection##History")) {
            selectedSnapshotIds.clear();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Selected: %zu", selectedSnapshotIds.size());

        ImGui::SameLine();
        if (ImGui::Button("Export Selected .md##History")) {
            auto selected = selectedSnapshots();
            if (selected.empty()) {
                ImGui::OpenPopup("HistoryExportSelectionEmpty");
            } else {
                exportMarkdown = buildMarkdown(selected);
                showExportDialog = true;
                exportSelector.Open(lastExportPath);
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Export Visible .md##History")) {
            exportMarkdown = buildMarkdown(snapshots);
            showExportDialog = true;
            exportSelector.Open(lastExportPath);
        }

        // Table
        if (ImGui::BeginTable("HistoryTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0, 300))) {
            ImGui::TableSetupColumn("Sel", ImGuiTableColumnFlags_WidthFixed, 40.0f);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 160.0f);
            ImGui::TableSetupColumn("Context", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Intent", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            bool triggerOpen = false;
            for (int i = 0; i < static_cast<int>(snapshots.size()); ++i) {
                const auto& snap = snapshots[i];
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                bool selected = selectedSnapshotIds.contains(snap.snapshotId);
                ImGui::PushID((snap.snapshotId + "_sel").c_str());
                if (ImGui::Checkbox("##SelectSnapshot", &selected)) {
                    if (selected) selectedSnapshotIds.insert(snap.snapshotId);
                    else selectedSnapshotIds.erase(snap.snapshotId);
                }
                ImGui::PopID();

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(snap.snapshotId.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(snap.inputContextSummary.c_str());

                ImGui::TableSetColumnIndex(3);
                ImGui::TextUnformatted(snap.intent.c_str());

                ImGui::TableSetColumnIndex(4);
                ImGui::PushID(i);
                if (ImGui::Button("View")) {
                    selectedIndex = i;
                    showDetailModal = true;
                    triggerOpen = true;
                }
                ImGui::PopID();
            }
            ImGui::EndTable();

            if (triggerOpen) {
                ImGui::OpenPopup("HistoryDetailModal");
            }
        }

        if (showDetailModal && selectedIndex >= 0 && selectedIndex < static_cast<int>(snapshots.size())) {
            ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
            if (ImGui::BeginPopupModal("HistoryDetailModal", &showDetailModal)) {
                const auto& snap = snapshots[selectedIndex];

                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "SNAPSHOT: %s", snap.snapshotId.c_str());
                ImGui::Separator();

                ImGui::TextDisabled("Intent: %s", snap.intent.c_str());
                ImGui::TextDisabled("Context:");
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
                ImGui::TextWrapped("%s", snap.inputContextSummary.c_str());
                ImGui::TextDisabled("Source Bundle:");
                ImGui::TextWrapped("%s", snap.sourceBundleId.c_str());
                ImGui::PopStyleColor();

                ImGui::Separator();
                ImGui::BeginChild("ContentScroll", ImVec2(0, -40));
                ImGui::TextWrapped("%s", snap.aiOutput.c_str());
                ImGui::EndChild();

                ImGui::Separator();
                if (ImGui::Button("Close", ImVec2(120, 0))) {
                    showDetailModal = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        if (exportSelector.draw(&showExportDialog, exportPathResult, ".md", true)) {
            try {
                std::ofstream out(exportPathResult);
                if (!out.is_open()) {
                    exportError = "Could not open destination file for writing.";
                    ImGui::OpenPopup("HistoryExportError");
                } else {
                    out << exportMarkdown;
                    out.close();
                    std::filesystem::path selected(exportPathResult);
                    if (selected.has_parent_path()) {
                        lastExportPath = selected.parent_path().string();
                    }
                    ImGui::OpenPopup("HistoryExportSuccess");
                }
            } catch (const std::exception& e) {
                exportError = e.what();
                ImGui::OpenPopup("HistoryExportError");
            }
        }

        if (ImGui::BeginPopupModal("HistoryExportSelectionEmpty", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(1, 0.6f, 0, 1), "No Snapshot Selected");
            ImGui::Text("Select one or more snapshots before exporting selected entries.");
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal("HistoryExportSuccess", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Markdown export completed successfully.");
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal("HistoryExportError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Failed to export markdown file.");
            if (!exportError.empty()) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", exportError.c_str());
            }
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }

private:
    static std::string nowIsoLike() {
        auto now = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&tt);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    static std::string buildMarkdown(const std::vector<Application::DTO::Cognitive::InterpretationSnapshotDTO>& snapshots) {
        std::ostringstream md;
        md << "# LLM Interpretation Evaluations\n\n";
        md << "Generated at: " << nowIsoLike() << "\n\n";
        md << "Total snapshots: " << snapshots.size() << "\n\n";

        for (const auto& snap : snapshots) {
            md << "## Snapshot " << snap.snapshotId << "\n\n";
            md << "- Created At: " << (snap.createdAt.empty() ? "unknown" : snap.createdAt) << "\n";
            md << "- Intent: " << snap.intent << "\n";
            md << "- Context: " << snap.inputContextSummary << "\n";
            md << "- Source Bundle: " << snap.sourceBundleId << "\n";
            md << "- Prompt Version: " << snap.promptVersion << "\n\n";
            md << "### Evaluation\n\n";
            md << snap.aiOutput << "\n\n";
            md << "---\n\n";
        }

        return md.str();
    }
};

} // namespace UI::Components
