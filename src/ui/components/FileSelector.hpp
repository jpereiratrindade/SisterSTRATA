#pragma once

#include "imgui.h"
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <system_error>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <array>
#include <cfloat>
#include <cstring>

namespace UI::Components {

namespace fs = std::filesystem;

/**
 * @brief Modal file browser built with ImGui.
 */
class FileBrowser {
public:
    FileBrowser() = default;

    /**
     * @brief Whether the browser popup is open.
     */
    bool IsOpen() const { return isOpen_; }

    /**
     * @brief Restrict selection to directories only.
     */
    void SetSelectDirectoriesOnly(bool v) { selectDirectoriesOnly_ = v; }

    /**
     * @brief Set the current browsing path.
     */
    void SetCurrentPath(const std::string& path) { currentPathStr_ = path; }

    /**
     * @brief Open the browser popup.
     * @param directoriesOnly If true, only directories can be selected.
     */
    void Open(bool directoriesOnly = false) {
        isOpen_ = true;
        selectDirectoriesOnly_ = directoriesOnly;
        EnsureValidCurrentPath();
        Refresh();
        ImGui::OpenPopup("File Browser");
    }

    /**
     * @brief Render the browser and return selected paths.
     * @param outPaths Output selection list.
     * @return true when a selection is confirmed.
     */
    bool Render(std::vector<std::string>& outPaths) {
        bool confirmed = false;
        if (!isOpen_) return false;

        EnsureValidCurrentPath();
        ImGui::SetNextWindowSize(ImVec2(800, 520), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(400, 300), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::BeginPopupModal("File Browser", &isOpen_, ImGuiWindowFlags_NoSavedSettings)) {
            // Navigation Buttons
            if (ImGui::Button("Root (/)")) {
                currentPathStr_ = "/";
                Refresh();
            }
            ImGui::SameLine();
            if (ImGui::Button("..")) {
                fs::path parent = fs::path(currentPathStr_).parent_path();
                if (!parent.empty() && parent.string() != currentPathStr_) {
                    currentPathStr_ = parent.string();
                    Refresh();
                }
            }
            ImGui::SameLine();

            // Editable Path Bar
            // We use a temporary buffer to allow editing, but strictly we should init it from currentPathStr_ every time 
            // the path changes externally. However, for a simple implementation, we copy current -> buf each frame 
            // unless user is editing. 
            // Better: Just use a local buffer initialized from currentPathStr_, let ID keep state?
            // Or simpler: InputText with std::string (if using imgui_stdlib.h, but we aren't sure).
            // We'll use a fixed buffer member for editing.
            
            static char pathBuffer[1024];
            std::strncpy(pathBuffer, currentPathStr_.c_str(), sizeof(pathBuffer));
            pathBuffer[sizeof(pathBuffer) - 1] = '\0';
            
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 220); // Reserve space for sort buttons
            if (ImGui::InputText("##Path", pathBuffer, sizeof(pathBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                std::string newPath = pathBuffer;
                // Basic validation
                std::error_code ec;
                if (fs::exists(newPath, ec) && fs::is_directory(newPath, ec)) {
                    currentPathStr_ = newPath;
                    Refresh();
                }
            }

            ImGui::Separator();
            ImGui::TextUnformatted("Sort by:");
            ImGui::SameLine();
            bool byName = !sortByDate_;
            if (ImGui::RadioButton("Name", byName)) {
                sortByDate_ = false;
                Refresh();
            }
            ImGui::SameLine();
            bool byDate = sortByDate_;
            if (ImGui::RadioButton("Date", byDate)) {
                sortByDate_ = true;
                Refresh();
            }
            ImGui::SameLine();
            if (ImGui::Button(sortAscending_ ? "Asc" : "Desc")) {
                sortAscending_ = !sortAscending_;
                Refresh();
            }

            ImVec2 listAvail = ImGui::GetContentRegionAvail();
            float listHeight = std::max(150.0f, listAvail.y - 60.0f);
            ImGui::BeginChild("Files", ImVec2(0, listHeight), true);

            bool ctrl = ImGui::GetIO().KeyCtrl;
            bool navigateToDir = false;
            std::string nextDir;

            if (entries_.empty()) {
                ImGui::TextDisabled("Empty directory or access denied.");
            }

            for (const auto& entry : entries_) {
                const bool isDir = entry.isDir;
                std::string displayName = DisplayNameFor(entry.pathStr);
                std::string label = (isDir ? "[DIR] " : "      ") + displayName;
                if (!entry.timeLabel.empty()) {
                    label += "  [" + entry.timeLabel + "]";
                }

                bool isSelected = IsSelected(entry.pathStr);
                bool clicked = ImGui::Selectable(
                    label.c_str(),
                    isSelected,
                    ImGuiSelectableFlags_AllowDoubleClick
                );

                if (clicked) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        if (isDir) {
                            nextDir = entry.pathStr;
                            navigateToDir = true;
                            selectedEntries_.clear();
                        } else if (!selectDirectoriesOnly_) {
                            outPaths.clear();
                            outPaths.push_back(entry.pathStr);
                            confirmed = true;
                            isOpen_ = false;
                            ImGui::CloseCurrentPopup();
                            break;
                        }
                    } else {
                        if (ctrl && !selectDirectoriesOnly_ && !isDir) {
                            ToggleSelection(entry.pathStr);
                        } else {
                            selectedEntries_.clear();
                            selectedEntries_.push_back(entry.pathStr);
                        }
                    }
                }
            }

            ImGui::EndChild();
            ImGui::Separator();

            if (navigateToDir && !nextDir.empty()) {
                currentPathStr_ = nextDir;
                Refresh();
            }

            if (ImGui::Button("Open")) {
                std::vector<std::string> valid;
                for (const auto& p : selectedEntries_) {
                    std::error_code ec;
                    fs::path sel(p);
                    if (selectDirectoriesOnly_) {
                        if (fs::exists(sel, ec) && fs::is_directory(sel, ec)) {
                            valid.push_back(p);
                        }
                    } else if (fs::exists(sel, ec) && !fs::is_directory(sel, ec)) {
                        valid.push_back(p);
                    }
                }
                if (!valid.empty()) {
                    outPaths = valid;
                    confirmed = true;
                    isOpen_ = false;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                isOpen_ = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        return confirmed;
    }

private:
    struct BrowserEntry {
        std::string pathStr;
        bool isDir = false;
        std::filesystem::file_time_type modTime{};
        std::string timeLabel;
    };

    static std::string DisplayNameFor(const std::string& pathStr) {
        if (pathStr.empty()) return "<vazio>";
        auto pos = pathStr.find_last_of("/\\");
        std::string base = (pos == std::string::npos) ? pathStr : pathStr.substr(pos + 1);
        if (base.empty()) base = pathStr;
        return base;
    }

    void EnsureValidCurrentPath() {
        std::error_code ec;
        fs::path candidate = currentPathStr_.empty() ? fs::current_path(ec) : fs::path(currentPathStr_);

        auto isValidDir = [](const fs::path& p) {
            if (p.empty()) return false;
            std::error_code lec;
            return fs::exists(p, lec) && fs::is_directory(p, lec);
        };

        if (ec || !isValidDir(candidate)) {
            const char* homeEnv = std::getenv("HOME");
            candidate = (homeEnv && *homeEnv) ? fs::path(homeEnv) : fs::path("/");
            if (!isValidDir(candidate)) {
                candidate = fs::path("/");
            }
        }

        std::error_code aec;
        fs::path abs = fs::absolute(candidate, aec);
        if (!aec && !abs.empty()) {
            candidate = abs;
        }

        currentPathStr_ = candidate.string();
    }

    void Refresh() {
        EnsureValidCurrentPath();
        entries_.clear();

        std::error_code ec;
        fs::directory_iterator it(currentPathStr_, ec);
        if (ec) {
            return;
        }

        fs::directory_iterator end;
        for (; it != end; it.increment(ec)) {
            if (ec) {
                break;
            }

            const auto& dirEntry = *it;
            std::string entryPath = dirEntry.path().string();
            if (entryPath.empty()) continue;

            BrowserEntry be;
            be.pathStr = entryPath;
            be.isDir = dirEntry.is_directory();

            std::error_code tec;
            auto ftime = dirEntry.last_write_time(tec);
            if (!tec) {
                be.modTime = ftime;
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now());
                std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
                char buf[64];
                std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", std::localtime(&tt));
                be.timeLabel = buf;
            }

            entries_.push_back(std::move(be));
        }

        std::sort(entries_.begin(), entries_.end(), [this](const BrowserEntry& a, const BrowserEntry& b) {
            if (a.isDir != b.isDir) return a.isDir > b.isDir;
            if (sortByDate_ && a.modTime != b.modTime) {
                return sortAscending_ ? (a.modTime < b.modTime) : (a.modTime > b.modTime);
            }
            auto na = DisplayNameFor(a.pathStr);
            auto nb = DisplayNameFor(b.pathStr);
            return sortAscending_ ? (na < nb) : (na > nb);
        });
    }

    std::string currentPathStr_;
    std::vector<std::string> selectedEntries_;
    std::vector<BrowserEntry> entries_;
    bool isOpen_ = false;
    bool selectDirectoriesOnly_ = false;
    bool sortByDate_ = false;
    bool sortAscending_ = true;

    bool IsSelected(const std::string& path) const {
        return std::find(selectedEntries_.begin(), selectedEntries_.end(), path) != selectedEntries_.end();
    }

    void ToggleSelection(const std::string& path) {
        auto it = std::find(selectedEntries_.begin(), selectedEntries_.end(), path);
        if (it == selectedEntries_.end()) {
            selectedEntries_.push_back(path);
        } else {
            selectedEntries_.erase(it);
        }
    }
};

/**
 * @brief File selector wrapper for open/save flows.
 */
class FileSelector {
public:
    FileSelector(const std::string& title = "Select File") : title_(title) {}

    /**
     * @brief Request opening the selector at a given path.
     */
    void Open(const std::string& startPath = "") {
        pendingPath_ = startPath;
        openRequested_ = true;
    }

    /**
     * @brief Draw the selector modal.
     * @param open Popup open flag.
     * @param result Output path on success.
     * @param extensionFilter Optional extension (e.g. ".csv").
     * @param allowCreate When true, shows save flow.
     * @return true when a path is confirmed.
     */
    bool draw(bool* open, std::string& result, const std::string& extensionFilter = "", bool allowCreate = false) {
        if (!open || !*open) return false;
        bool selected = false;

        if (openRequested_) {
            if (allowCreate) {
                // In Save mode, we want the Modal to open.
                // But the browser is secondary.
                ImGui::OpenPopup(title_.c_str());
            } else {
                // In Open mode, we just open the browser directly.
                if (!pendingPath_.empty()) {
                    browser_.SetCurrentPath(pendingPath_);
                    setFilenameFromPath(pendingPath_);
                }
                browser_.Open(allowCreate);
            }
            openRequested_ = false;
        }

        if (allowCreate) {
            // Save Dialog Logic
            if (ImGui::BeginPopupModal(title_.c_str(), open, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Directory:");
                ImGui::SameLine();
                ImGui::InputText("##dir", inputDir_.data(), inputDir_.size());
                ImGui::SameLine();
                if (ImGui::Button("Browse...")) {
                    // Open browser to pick directory
                    browser_.Open(true);
                }

                ImGui::Text("Filename:");
                ImGui::SameLine();
                ImGui::InputText("##filename", inputName_.data(), inputName_.size());

                bool hasPath = inputDir_[0] != '\0' && inputName_[0] != '\0';
                if (!hasPath) ImGui::BeginDisabled();
                if (ImGui::Button("Save", ImVec2(120, 0))) {
                    fs::path out = std::string(inputDir_.data());
                    out /= std::string(inputName_.data());
                    if (!extensionFilter.empty() && out.extension() != extensionFilter) {
                        out += extensionFilter;
                    }
                    result = fs::absolute(out).string();
                    selected = !result.empty();
                    *open = false;
                    ImGui::CloseCurrentPopup();
                }
                if (!hasPath) ImGui::EndDisabled();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    *open = false;
                    ImGui::CloseCurrentPopup();
                }
                
                // Handle Browser inside the Save Dialog context
                std::vector<std::string> chosen;
                if (browser_.Render(chosen)) {
                    if (!chosen.empty()) {
                        setDirectory(chosen.front());
                    }
                }
                
                ImGui::EndPopup();
            }
        } else {
            // Open Dialog Logic
            std::vector<std::string> chosen;
            if (browser_.Render(chosen)) {
                if (!chosen.empty()) {
                    result = chosen.front();
                    selected = true;
                    *open = false;
                }
            } else if (!browser_.IsOpen()) {
                // If browser closed without selection (e.g. cancelled), close wrapper
                *open = false; 
            }
        }

        return selected;
    }

private:
    void setDirectory(const std::string& dir) {
        inputDir_.fill('\0');
        std::snprintf(inputDir_.data(), inputDir_.size(), "%s", dir.c_str());
    }

    void setFilenameFromPath(const std::string& path) {
        fs::path p(path);
        if (p.has_parent_path()) {
            setDirectory(p.parent_path().string());
        }
        if (p.has_filename()) {
            inputName_.fill('\0');
            std::snprintf(inputName_.data(), inputName_.size(), "%s", p.filename().string().c_str());
        }
    }

    std::string title_;
    FileBrowser browser_;
    std::string pendingPath_;
    std::array<char, 512> inputDir_ = {};
    std::array<char, 256> inputName_ = {};
    bool openRequested_ = false;
};

} // namespace UI::Components
