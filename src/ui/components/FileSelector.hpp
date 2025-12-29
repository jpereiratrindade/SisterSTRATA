#pragma once

#include "imgui.h"
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <functional>
#include <array>
#include <cstdio>

namespace UI::Components {

class FileSelector {
public:
    FileSelector(const std::string& title = "Select File") 
        : title_(title) {
        // Lazy init: Do not access filesystem in constructor to prevent init-order crashes
    }

    // Call this to open the popup. 
    // Optionally provide a path to start in (if empty, uses current or cached).
    void Open(const std::string& startPath = "") {
        if (!startPath.empty()) {
            SetPath(startPath);
        } else if (currentPath_.empty()) {
            currentPath_ = std::filesystem::current_path();
            refresh();
        } else {
            refresh();
        }
        openRequested_ = true;
    }

    // Returns true if a file was selected. The selected path is stored in 'result'.
    // When allowCreate is true, shows a filename input for Save As behavior.
    bool draw(bool* open, std::string& result, const std::string& extensionFilter = "", bool allowCreate = false) {
        if (!open || !*open) return false;

        ensureCurrentPath();
        bool selected = false;
        if (openRequested_) {
            ImGui::OpenPopup(title_.c_str());
            openRequested_ = false;
        }

        if (ImGui::BeginPopupModal(title_.c_str(), open, ImGuiWindowFlags_AlwaysAutoResize)) {
            // Header: Current Path
            ImGui::Text("Path: %s", currentPath_.string().c_str());
            if (ImGui::Button("Up")) {
                if (currentPath_.has_parent_path()) {
                    currentPath_ = currentPath_.parent_path();
                    refresh();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Refresh")) {
                refresh();
            }

            ImGui::Separator();

            // File List
            ImGui::BeginChild("Files", ImVec2(400, 300), true);
            
            // Directories first
            for (const auto& entry : directories_) {
                if (ImGui::Selectable(("[D] " + entry.filename().string()).c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        currentPath_ = entry;
                        refresh();
                    }
                }
            }

            ImGui::Separator();

            // Files
            for (const auto& entry : files_) {
                if (!extensionFilter.empty() && entry.extension() != extensionFilter) continue;

                if (ImGui::Selectable(entry.filename().string().c_str(), selectedFile_ == entry, ImGuiSelectableFlags_AllowDoubleClick)) {
                    selectedFile_ = entry;
                    if (allowCreate) {
                        setInputName(entry.filename().string());
                    } else if (ImGui::IsMouseDoubleClicked(0)) {
                        result = std::filesystem::absolute(entry).string();
                        selected = true;
                        *open = false;
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
            ImGui::EndChild();

            // Footer
            ImGui::Separator();
            if (allowCreate) {
                ImGui::Text("Filename:");
                ImGui::SameLine();
                ImGui::InputText("##filename", inputName_.data(), inputName_.size());
            }

            bool hasSaveName = allowCreate && inputName_[0] != '\0';
            if ((!allowCreate && selectedFile_.empty()) || (allowCreate && !hasSaveName && selectedFile_.empty())) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button(allowCreate ? "Save" : "Open", ImVec2(120, 0))) {
                if (allowCreate) {
                    if (!hasSaveName && selectedFile_.empty()) {
                        // Ignore clicks when no target is selected.
                    } else {
                    std::filesystem::path out = currentPath_;
                    if (hasSaveName) {
                        out /= std::string(inputName_.data());
                    } else {
                        out = selectedFile_;
                    }
                        if (out.empty()) {
                            out = std::filesystem::current_path();
                        }
                        result = std::filesystem::absolute(out).string();
                    }
                } else {
                    result = std::filesystem::absolute(selectedFile_).string();
                }
                if (!result.empty()) {
                    selected = true;
                    *open = false;
                    ImGui::CloseCurrentPopup();
                }
            }
            if ((!allowCreate && selectedFile_.empty()) || (allowCreate && !hasSaveName && selectedFile_.empty())) {
                ImGui::EndDisabled();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                *open = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        return selected;
    }

    void SetPath(const std::string& path) {
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
            currentPath_ = path;
        } else {
            // If file, go to parent; if invalid, fall back to cwd.
            std::filesystem::path p(path);
            if (p.has_filename()) {
                setInputName(p.filename().string());
            }
            if (std::filesystem::exists(p) && p.has_parent_path()) {
                currentPath_ = p.parent_path();
            } else if (p.has_parent_path() && std::filesystem::exists(p.parent_path())) {
                currentPath_ = p.parent_path();
            } else {
                currentPath_ = std::filesystem::current_path();
            }
        }
        refresh();
    }

private:
    void refresh() {
        if (currentPath_.empty()) {
            try {
                currentPath_ = std::filesystem::current_path();
            } catch (...) {
                return;
            }
        }
        directories_.clear();
        files_.clear();
        selectedFile_.clear();

        try {
            for (const auto& entry : std::filesystem::directory_iterator(currentPath_)) {
                if (entry.is_directory()) {
                    directories_.push_back(entry.path());
                } else {
                    files_.push_back(entry.path());
                }
            }
            std::sort(directories_.begin(), directories_.end());
            std::sort(files_.begin(), files_.end());
        } catch (...) {}
    }

    std::string title_;
    std::filesystem::path currentPath_;
    std::vector<std::filesystem::path> directories_;
    std::vector<std::filesystem::path> files_;
    std::filesystem::path selectedFile_;
    std::array<char, 256> inputName_ = {};
    bool openRequested_ = false;

    void setInputName(const std::string& name) {
        inputName_.fill('\0');
        std::snprintf(inputName_.data(), inputName_.size(), "%s", name.c_str());
    }

    void ensureCurrentPath() {
        if (!currentPath_.empty()) return;
        try {
            currentPath_ = std::filesystem::current_path();
            refresh();
        } catch (...) {
        }
    }
};

} // namespace UI::Components
