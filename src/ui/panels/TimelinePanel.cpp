#include "TimelinePanel.hpp"
#include "core/domain/fourth_dimension/TrajectoryService.hpp"
#include "core/domain/fourth_dimension/CoherenceIntensityService.hpp"
#include "world3d/World3D.hpp"
#include "imgui.h"
#include <algorithm>
#include <vector>

namespace UI::Panels {

void TimelinePanel::draw(bool* open) {
    if (!open || !*open) return;

    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Fourth Dimension (Resilience)", open)) {
        ImGui::End();
        return;
    }

    if (!trajectory_ || !vegPanel_) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[!] Missing Dependencies");
        ImGui::End();
        return;
    }

    // Capture Controls
    if (ImGui::Button("Capture Current State", ImVec2(-1, 0))) {
        const auto* res = vegPanel_->getLastScenarioResult();
        if (res && !res->classification.empty()) {
            std::string meta = "State " + std::to_string(trajectory_->getNextOrdinal());
            
            // Dummy Water Mask for now (requires deeper integration with Hydro)
            std::vector<bool> waterMask; 
            
            Core::Domain::FourthDimension::TrajectoryService::captureState(
                *trajectory_,
                res->classification, // Indices
                vegPanel_->getSystem(), // System (for lookup)
                waterMask,
                meta
            );
        } else {
            // Warn
             ImGui::OpenPopup("CaptureFailed");
        }
    }

    if (ImGui::BeginPopup("CaptureFailed")) {
        ImGui::Text("Cannot capture state.");
        ImGui::Text("Please run 'Resolve Scenario' first.");
        if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Separator();
    
    // Ghost Mode Warning
    if (ghostMode_) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 1.0f, 1.0f, 1.0f));
        ImGui::Text("[GHOST MODE ACTIVE]");
        ImGui::PopStyleColor();
        ImGui::TextWrapped("Viewing past state. Simulation is read-only.");
        
        if (ImGui::Button("Exit Ghost Mode (Restore Soil)")) {
            ghostMode_ = false;
            World3D::resetVisualization();
        }
        ImGui::Separator();
    }

    // Trajectory List
    ImGui::Text("Trajectory (%zu states)", trajectory_->getTimeSlices().size());
    
    ImGui::BeginChild("TrajectoryList", ImVec2(0, 0), true);
    auto& slices = trajectory_->getTimeSlices();
    
    for (int i = 0; i < (int)slices.size(); ++i) {
        const auto& slice = slices[i];
        
        std::string label = "T" + std::to_string(slice.getOrdinalIndex()) + ": " + slice.getMetadata();
        bool isSelected = (selectedSliceIndex_ == i);
        
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            selectedSliceIndex_ = i;
            // Activate Ghost Mode immediately on selection?
            // Or require explicit "View"?
            // Let's do explicit View to avoid flashing.
        }
    }
    ImGui::EndChild();

    // Selected Item Actions
    if (selectedSliceIndex_ >= 0 && selectedSliceIndex_ < (int)slices.size()) {
        const auto& slice = slices[selectedSliceIndex_];
        ImGui::Separator();
        ImGui::Text("Selected: T%d", slice.getOrdinalIndex());
        
        if (ImGui::Button("View (Ghost)")) {
            ghostMode_ = true;
            applyGhostVisualization(slice);
        }
    }

    ImGui::Separator();
    ImGui::Text("Coherence Intensity Map");

    if (slices.size() < 2) {
        ImGui::TextDisabled("Need at least 2 states to compare.");
    } else {
        if (compareSliceA_ < 0 || compareSliceA_ >= (int)slices.size()) compareSliceA_ = 0;
        if (compareSliceB_ < 0 || compareSliceB_ >= (int)slices.size()) compareSliceB_ = std::min(1, (int)slices.size() - 1);

        auto sliceLabel = [](const Core::Domain::FourthDimension::TimeSlice& slice) {
            return "T" + std::to_string(slice.getOrdinalIndex()) + ": " + slice.getMetadata();
        };

        std::string labelA = sliceLabel(slices[compareSliceA_]);
        std::string labelB = sliceLabel(slices[compareSliceB_]);

        if (ImGui::BeginCombo("Slice A", labelA.c_str())) {
            for (int i = 0; i < (int)slices.size(); ++i) {
                bool isSelected = (compareSliceA_ == i);
                std::string label = sliceLabel(slices[i]);
                if (ImGui::Selectable(label.c_str(), isSelected)) {
                    compareSliceA_ = i;
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Slice B", labelB.c_str())) {
            for (int i = 0; i < (int)slices.size(); ++i) {
                bool isSelected = (compareSliceB_ == i);
                std::string label = sliceLabel(slices[i]);
                if (ImGui::Selectable(label.c_str(), isSelected)) {
                    compareSliceB_ = i;
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Compute Coherence Map")) {
            coherenceStatus_.clear();
            lastCoherenceMean_ = -1.0f;

            if (compareSliceA_ == compareSliceB_) {
                coherenceStatus_ = "Select two different states.";
            } else {
                const auto& sliceA = slices[compareSliceA_];
                const auto& sliceB = slices[compareSliceB_];
                const auto& coverA = sliceA.getEcologicalCoverState();
                const auto& coverB = sliceB.getEcologicalCoverState();

                if (coverA.size() != coverB.size() || coverA.empty()) {
                    coherenceStatus_ = "State sizes do not match.";
                } else {
                    Core::Domain::FourthDimension::CoherenceIntensityParams params;
                    const auto& hydro = World3D::getHydroGrid();
                    if (hydro.isValid() && hydro.flowAccumulationCells.size() == coverA.size()) {
                        params.width = hydro.width;
                        params.height = hydro.height;
                    }
                    params.radius = 2;
                    params.sigma = 1.0f;
                    params.weightType = 0.45f;
                    params.weightStructure = 0.4f;
                    params.weightEdge = 0.15f;
                    params.ignoreNoData = true;
                    params.ignoreWaterMask = true;

                    auto map = Core::Domain::FourthDimension::CoherenceIntensityService::compare(sliceA, sliceB, params);
                    if (map.intensity.empty()) {
                        coherenceStatus_ = "Unable to compute map (grid dimensions missing or invalid).";
                    } else {
                        double sum = 0.0;
                        for (float v : map.intensity) sum += v;
                        lastCoherenceMean_ = static_cast<float>(sum / map.intensity.size());
                        coherenceStatus_ = "Coherence map computed.";
                    }
                }
            }
        }

        if (!coherenceStatus_.empty()) {
            ImGui::TextWrapped("%s", coherenceStatus_.c_str());
        }
        if (lastCoherenceMean_ >= 0.0f) {
            ImGui::Text("Mean Intensity: %.3f", lastCoherenceMean_);
        }

        // --- Cognitive Insight (Qwen) ---
        ImGui::Separator();
        ImGui::Text("Cognitive Insight (Qwen)");

        if (llmService_ && lastCoherenceMean_ >= 0.0f) {
            if (ImGui::Button("Solicitar Análise Hermenêutica", ImVec2(-1, 0)) && !requestInProgress_) {
                requestInProgress_ = true;
                cognitiveInsight_.clear();
                llmErrorMessage_.clear();

                // Build Context for LLM
                std::vector<Application::Ports::LLMMessage> messages;
                // Note: The System Prompt is loaded by the adapter/orchestrator or passed explicitly here.
                // For this PoC, we rely on the adapter knowing its role.
                
                std::string prompt = "Analise a transição entre " + labelA + " e " + labelB + ". ";
                prompt += "Métrica de Intensidade de Coerência Média: " + std::to_string(lastCoherenceMean_) + ".";
                
                messages.push_back({Application::Ports::LLMRole::User, prompt});

                llmService_->requestCompletion(messages, [this](const Application::Ports::ILLMService::Response& res) {
                    std::lock_guard<std::mutex> lock(insightMutex_);
                    if (res.success) {
                        cognitiveInsight_ = res.content;
                    } else {
                        llmErrorMessage_ = res.errorMessage;
                    }
                    requestInProgress_ = false;
                });
            }

            if (requestInProgress_) {
                ImGui::TextDisabled("Processando interpretação...");
                // Add a simple animated spinner or dots if needed.
            }

            if (!llmErrorMessage_.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Erro: %s", llmErrorMessage_.c_str());
            }

            {
                std::lock_guard<std::mutex> lock(insightMutex_);
                if (!cognitiveInsight_.empty()) {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
                    
                    // Adjustable height logic
                    ImGui::BeginChild("InsightOutput", ImVec2(0, insightWindowHeight_), true);
                    ImGui::TextWrapped("%s", cognitiveInsight_.c_str());
                    ImGui::EndChild();
                    
                    // Resize Handle (Invisible button that captures drag)
                    ImGui::Button("###ResizeHandleInsight", ImVec2(-1, 4.0f));
                    if (ImGui::IsItemActive()) {
                        insightWindowHeight_ += ImGui::GetIO().MouseDelta.y;
                        if (insightWindowHeight_ < 50.0f) insightWindowHeight_ = 50.0f;
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

                    ImGui::PopStyleColor();

                    if (ImGui::Button("Save Hermeneutic Analysis", ImVec2(-1, 0))) {
                        saveAnalysisToFile();
                    }
                }
            }
        } else if (!llmService_) {
             ImGui::TextDisabled("Serviço de IA não disponível.");
        } else {
             ImGui::TextDisabled("Calcule a coerência para habilitar o Insight.");
        }
    }

    ImGui::End();
}

void TimelinePanel::applyGhostVisualization(const Core::Domain::FourthDimension::TimeSlice& slice) {
    World3D::applyClassificationVisualization(slice.getEcologicalCoverState());
}

void TimelinePanel::saveAnalysisToFile() {
    std::string filename = "hermeneutic_analysis_" + std::to_string(std::time(nullptr)) + ".txt";
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "==================================================\n";
        file << "SISTERSTRATA HERMENEUTIC ANALYSIS\n";
        file << "==================================================\n\n";
        file << cognitiveInsight_ << "\n\n";
        file << "--------------------------------------------------\n";
        file << "Timestamp: " << std::time(nullptr) << "\n";
        file << "==================================================\n";
        file.close();
        std::cout << "[TimelinePanel] Analysis saved to: " << filename << std::endl;
    } else {
        std::cerr << "[TimelinePanel] Failed to save analysis to: " << filename << std::endl;
    }
}

} // namespace UI::Panels
