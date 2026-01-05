#include "TimelinePanel.hpp"
#include "application/ports/IFileSystem.hpp"
#include "core/domain/fourth_dimension/TrajectoryService.hpp"
#include "core/domain/fourth_dimension/CoherenceIntensityService.hpp"
#include "core/domain/fourth_dimension/patch_trajectory/PatchTrajectoryService.hpp"
#include "core/domain/spatial_pattern/PatchAnalysis.hpp"
#include "world3d/World3D.hpp"
#include "imgui.h"
#include <fstream>
#include <ctime>
#include <vector>
#include <iostream>
#include <map>
#include <iomanip>
#include <sstream>

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
            
            std::vector<bool> waterMask; 
            
            Core::Domain::FourthDimension::TrajectoryService::captureState(
                *trajectory_,
                res->classification, 
                vegPanel_->getSystem(), 
                waterMask,
                meta
            );
        } else {
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
    
    ImGui::BeginChild("TrajectoryList", ImVec2(0, 150), true);
    auto& slices = trajectory_->getTimeSlices();
    
    for (int i = 0; i < (int)slices.size(); ++i) {
        const auto& slice = slices[i];
        std::string label = "T" + std::to_string(slice.getOrdinalIndex()) + ": " + slice.getMetadata();
        bool isSelected = (selectedSliceIndex_ == i);
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            selectedSliceIndex_ = i;
        }
    }
    ImGui::EndChild();

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

        const auto& sliceA = slices[compareSliceA_];
        const auto& sliceB = slices[compareSliceB_];
        std::string labelA = sliceLabel(sliceA);
        std::string labelB = sliceLabel(sliceB);

        if (ImGui::BeginCombo("Slice A", labelA.c_str())) {
            for (int i = 0; i < (int)slices.size(); ++i) {
                bool isSelected = (compareSliceA_ == i);
                std::string label = sliceLabel(slices[i]);
                if (ImGui::Selectable(label.c_str(), isSelected)) compareSliceA_ = i;
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Slice B", labelB.c_str())) {
            for (int i = 0; i < (int)slices.size(); ++i) {
                bool isSelected = (compareSliceB_ == i);
                std::string label = sliceLabel(slices[i]);
                if (ImGui::Selectable(label.c_str(), isSelected)) compareSliceB_ = i;
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Compute Coherence Map")) {
            coherenceStatus_.clear();
            lastCoherenceMean_ = -1.0f;

            if (compareSliceA_ == compareSliceB_) {
                coherenceStatus_ = "Select two different states.";
            } else {
                const auto& coverA = sliceA.getEcologicalCoverState();
                const auto& coverB = sliceB.getEcologicalCoverState();

                if (coverA.size() != coverB.size() || coverA.empty()) {
                    coherenceStatus_ = "State sizes do not match.";
                } else {
                    Core::Domain::FourthDimension::CoherenceIntensityParams params;
                    const auto& hydro = World3D::getHydroGrid();
                    if (hydro.isValid() && (int)hydro.flowAccumulationCells.size() == (int)coverA.size()) {
                        params.width = hydro.width;
                        params.height = hydro.height;
                    }
                    params.radius = 2;
                    params.sigma = 1.0f;
                    params.weightType = 0.45f;
                    params.weightStructure = 0.4f;
                    params.weightEdge = 0.15f;

                    auto map = Core::Domain::FourthDimension::CoherenceIntensityService::compare(sliceA, sliceB, params);
                    if (map.intensity.empty()) {
                        coherenceStatus_ = "Unable to compute map.";
                    } else {
                        double sum = 0.0;
                        for (float v : map.intensity) sum += v;
                        lastCoherenceMean_ = static_cast<float>(sum / map.intensity.size());
                        coherenceStatus_ = "Coherence map computed.";
                    }
                }
            }
        }

        if (!coherenceStatus_.empty()) ImGui::TextWrapped("%s", coherenceStatus_.c_str());
        if (lastCoherenceMean_ >= 0.0f) ImGui::Text("Mean Intensity: %.3f", lastCoherenceMean_);
    }

    // --- Cognitive Insight (Qwen) Section ---
    ImGui::Separator();
    ImGui::Text("Cognitive Insight (Qwen)");

    if (llmService_) {
        // --- SEÇÃO 1: Análise de Transição (Hermenêutica Parcial) ---
        ImGui::Separator();
        ImGui::Text("Insight Hermenêutico (Transição)");
        
        if (lastCoherenceMean_ >= 0.0f) {
            if (ImGui::Button("Interpretar Transição A-B", ImVec2(-1, 0)) && !hermeneuticInProgress_) {
                hermeneuticInProgress_ = true;
                hermeneuticInsight_.clear();
                llmErrorMessage_.clear();

                const auto& sA = slices[compareSliceA_];
                const auto& sB = slices[compareSliceB_];
                std::string distA = getClassDistribution(sA);
                std::string distB = getClassDistribution(sB);

                std::string prompt = "Analise a transição ecológica entre (" + sA.getMetadata() + " e " + sB.getMetadata() + ").\n";
                prompt += "Composição A: " + distA + "\n";
                prompt += "Composição B: " + distB + "\n";
                prompt += "Métrica SSI: " + std::to_string(lastCoherenceMean_) + "\n";
                prompt += "Interprete esta mudança do ponto de vista da resiliência ecológica.";

                std::vector<Application::Ports::LLMMessage> messages;
                messages.push_back({Application::Ports::LLMRole::User, prompt});

                llmService_->requestCompletion(messages, [this](const Application::Ports::ILLMService::Response& res) {
                    std::lock_guard<std::mutex> lock(insightMutex_);
                    if (res.success) hermeneuticInsight_ = res.content;
                    else llmErrorMessage_ = res.errorMessage;
                    hermeneuticInProgress_ = false;
                });
            }
        } else {
            ImGui::TextDisabled("Calcule o mapa de coerência para habilitar.");
        }

        if (hermeneuticInProgress_) ImGui::TextDisabled("Analisando transição...");
        if (!hermeneuticInsight_.empty()) {
            ImGui::BeginChild("HermeneuticOutput", ImVec2(0, 100), true);
            ImGui::TextWrapped("%s", hermeneuticInsight_.c_str());
            ImGui::EndChild();
            if (ImGui::Button("Salvar Análise de Transição")) saveAnalysisToFile(hermeneuticInsight_, "transition");
        }

        // --- SEÇÃO 2: Análise de Trajetória Multi-Estado ---
        ImGui::Separator();
        ImGui::Text("Análise de Trajetória (DDD v1.1)");
        
        // Patch Discovery Logic
        if (selectedSliceIndex_ >= 0) {
            const auto& slice = slices[selectedSliceIndex_];
            if (!slice.getEcologicalCoverState().empty()) {
                 // Simple count discovery if not proxy
                 if (!slice.isProxy()) {
                    std::map<int, size_t> counts;
                    for (int code : slice.getEcologicalCoverState()) if (code > 0) counts[code]++;
                    lastPatchCount_ = (int)counts.size();
                 }
            }
            ImGui::Text("Patches detectados no estado selecionado: %d", lastPatchCount_);
        }

        ImGui::InputInt("ID do Patch", &selectedPatchId_);
        
        ImGui::BeginDisabled(trajectoryInProgress_);
        if (ImGui::Button("Analisar Trajetória do Patch", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
            if (selectedPatchId_ < 0) {
                llmErrorMessage_ = "ID do patch inválido.";
            } else {
                trajectoryInProgress_ = true;
                trajectoryInsight_.clear();
                
                using namespace Core::Domain::FourthDimension::PatchTrajectory;
                PatchTrajectory pt(selectedPatchId_);
                
                // Historical aggregation logic
                auto& slicesRef = trajectory_->getTimeSlices();
                double refX = -1.0, refY = -1.0;
                
                for (size_t i = 0; i < slicesRef.size(); ++i) {
                    auto& slice = slicesRef[i];
                    if (slice.isProxy()) Core::Domain::FourthDimension::TrajectoryPersistenceService::loadFromDisk(slice);
                    
                    const auto& cover = slice.getEcologicalCoverState();
                    if (cover.empty()) continue;

                    Core::Domain::SpatialPattern::GridData grid;
                    grid.values = std::vector<double>(cover.begin(), cover.end());
                    grid.width = (int)std::sqrt(grid.values.size()); grid.height = grid.width;
                    
                    auto result = Core::Domain::SpatialPattern::AnalyzeGrid(grid, {0.0, true, true});
                    
                    int bestPatchIdx = -1;
                    if (refX < 0) {
                        if (selectedPatchId_ > 0 && selectedPatchId_ <= (int)result.patches.size()) {
                            bestPatchIdx = selectedPatchId_ - 1;
                            refX = result.patches[bestPatchIdx].centroidX;
                            refY = result.patches[bestPatchIdx].centroidY;
                        }
                    } else {
                        double minVal = 50.0; 
                        for (int j = 0; j < (int)result.patches.size(); ++j) {
                            double dist = std::sqrt(std::pow(result.patches[j].centroidX - refX, 2) + std::pow(result.patches[j].centroidY - refY, 2));
                            if (dist < minVal) { minVal = dist; bestPatchIdx = j; }
                        }
                        if (bestPatchIdx >= 0) {
                            refX = result.patches[bestPatchIdx].centroidX;
                            refY = result.patches[bestPatchIdx].centroidY;
                        }
                    }

                    if (bestPatchIdx >= 0) {
                        const auto& pm = result.patches[bestPatchIdx];
                        PatchState ps;
                        ps.ordinalIndex = (int)i;
                        ps.area = (float)pm.area;
                        ps.perimeter = (float)pm.perimeter;
                        ps.shapeIndex = (float)pm.shape_index;
                        ps.adjacencyByClass[1] = 50.0f; // Placeholder until full adjacency logic is in
                        pt.addState(ps);
                    }
                }

                auto nameResolver = [this](int code) -> std::string {
                    if (vegPanel_) {
                        const auto& system = vegPanel_->getSystem();
                        if (code >= 0 && code < (int)system.getHypotheses().size()) {
                            const auto& hyp = system.getHypotheses()[code];
                            return hyp.getId().getValue() + " (" + hyp.getType().toString() + ")";
                        }
                    }
                    return "Classe " + std::to_string(code);
                };

                std::string summary = PatchTrajectoryService::generateLLMSummary(pt, nameResolver);
                std::string prompt = "Analise a trajetória histórica do patch ID " + std::to_string(selectedPatchId_) + ".\n" + summary;
                
                llmService_->requestCompletion({{Application::Ports::LLMRole::User, prompt}}, [this](const auto& res) {
                    std::lock_guard<std::mutex> lock(insightMutex_);
                    if (res.success) trajectoryInsight_ = res.content;
                    else llmErrorMessage_ = res.errorMessage;
                    trajectoryInProgress_ = false;
                });
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Análise Tática Global", ImVec2(-1, 0))) {
            trajectoryInProgress_ = true;
            trajectoryInsight_.clear();
            
            std::string prompt = "Realize uma análise tática global das trajetórias de manchas neste cenário.\n";
            prompt += "Atualmente existem " + std::to_string(slices.size()) + " estados temporais.\n\n";
            
            prompt += "Resumo da Composição da Paisagem por Estado:\n";
            for (size_t i = 0; i < slices.size(); ++i) {
                prompt += "Estado " + std::to_string(i) + " (" + slices[i].getMetadata() + "): " + getClassDistribution(slices[i]) + "\n";
            }
            
            prompt += "\nCom base nestas mudanças de composição, identifique tendências de fragmentação, regeneração ou estabilidade pulsátil.";
            prompt += " Analise como a estrutura espacial está evoluindo e se o sistema demonstra resiliência.";
            
            llmService_->requestCompletion({{Application::Ports::LLMRole::User, prompt}}, [this](const auto& res) {
                std::lock_guard<std::mutex> lock(insightMutex_);
                if (res.success) trajectoryInsight_ = res.content;
                else llmErrorMessage_ = res.errorMessage;
                trajectoryInProgress_ = false;
            });
        }
        ImGui::EndDisabled();

        if (trajectoryInProgress_) ImGui::TextDisabled("Calculando trajetórias...");
        if (!trajectoryInsight_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.15f, 0.1f, 1.0f));
            ImGui::BeginChild("TrajectoryOutput", ImVec2(0, insightWindowHeight_), true);
            ImGui::TextWrapped("%s", trajectoryInsight_.c_str());
            ImGui::EndChild();
            ImGui::PopStyleColor();
            
            if (ImGui::Button("Salvar Resumo de Trajetória")) saveAnalysisToFile(trajectoryInsight_, "trajectory");
        }
        
        if (!llmErrorMessage_.empty()) ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[Erro] %s", llmErrorMessage_.c_str());

    } else {
        ImGui::TextDisabled("Serviço de IA não disponível.");
    }

    ImGui::End();
}

void TimelinePanel::applyGhostVisualization(const Core::Domain::FourthDimension::TimeSlice& slice) {
    World3D::applyClassificationVisualization(slice.getEcologicalCoverState());
}

void TimelinePanel::saveAnalysisToFile(const std::string& content, const std::string& type) {
    std::string filename = "hermeneutic_" + type + "_" + std::to_string(std::time(nullptr)) + ".txt";
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "SISTERSTRATA HERMENEUTIC ANALYSIS (" << type << ")\n";
        file << "==========================================\n\n";
        file << content << "\n";
        file.close();
    }
}

std::string TimelinePanel::getClassDistribution(const Core::Domain::FourthDimension::TimeSlice& slice) {
    const auto& cover = slice.getEcologicalCoverState();
    if (cover.empty()) return "Sem dados";
    std::map<int, size_t> counts;
    for (int code : cover) counts[code]++;
    std::stringstream ss;
    ss << "[";
    bool first = true;
    for (auto const& [code, count] : counts) {
        if (!first) ss << ", ";
        float pct = (static_cast<float>(count) / cover.size()) * 100.0f;
        std::string name = "Classe_" + std::to_string(code);
        if (vegPanel_) {
            const auto& system = vegPanel_->getSystem();
            if (code >= 0 && code < (int)system.getHypotheses().size()) {
                const auto& hyp = system.getHypotheses()[code];
                name = hyp.getId().getValue() + " (" + hyp.getType().toString() + ")";
            }
        }
        ss << name << ": " << std::fixed << std::setprecision(1) << pct << "%";
        first = false;
    }
    ss << "]";
    return ss.str();
}


} // namespace UI::Panels
