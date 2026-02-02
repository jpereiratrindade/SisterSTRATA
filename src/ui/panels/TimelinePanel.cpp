#include "TimelinePanel.hpp"
#include "application/ports/IFileSystem.hpp"
#include "application/mappers/CognitiveMappers.hpp"
#include "ui/components/InterpretationModal.hpp"
#include "application/services/FourthDimensionService.hpp"
#include "application/services/World3DService.hpp"
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

    // Check for Deferred AI Results (Thread-safe UI update)
    if (session_) {
        std::lock_guard<std::mutex> lock(insightMutex_);
        if (aiResultReady_) {
            lastAiSnapshot_ = stagedAiSnapshot_; // Safe copy
            ImGui::OpenPopup("AI Cognitive Interpretation");
            aiResultReady_ = false;
        }
    }

    if (!trajectory_ || !vegPanel_) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[!] Missing Dependencies");
        ImGui::End();
        return;
    }

    // --- Mouse Picking for Patch ID Selection ---
    if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 vpSize = ImGui::GetMainViewport()->Size;
        
        int vertexIdx = Application::Services::World3DService::getPickIndex(mousePos.x, mousePos.y, (int)vpSize.x, (int)vpSize.y);
        if (vertexIdx != -1 && !lastPatchAnalysis_.labelImage.labels.empty()) {
            if (vertexIdx < (int)lastPatchAnalysis_.labelImage.labels.size()) {
                int patchId = lastPatchAnalysis_.labelImage.labels[vertexIdx];
                if (patchId > 0) {
                    selectedPatchId_ = patchId;
                    Application::Services::World3DService::highlightPatch(lastPatchAnalysis_.labelImage.labels, patchId);
                }
            }
        }
    }

    // Capture Controls
    if (ImGui::Button("Capture Current State", ImVec2(-1, 0))) {
        const auto& semantic = vegPanel_->getLastSemanticClassification();
        const auto* res = vegPanel_->getLastScenarioResult();
        const bool canSemantic = !semantic.empty();
        const bool canScenario = res && !res->classification.empty();
        const bool canScenarioSemantic = res && !res->semanticCodes.empty();
        const bool preferSemantic = vegPanel_->isSemanticClassificationActive() || canScenarioSemantic || !canScenario;

        if (preferSemantic && (canSemantic || canScenarioSemantic)) {
            std::string meta = "Semantic State " + std::to_string(trajectory_->getNextOrdinal());
            std::vector<bool> waterMask;
            Application::Services::FourthDimensionService::captureSemanticState(
                *trajectory_,
                canScenarioSemantic ? res->semanticCodes : semantic,
                waterMask,
                meta
            );
        } else if (canScenario) {
            std::string meta = "State " + std::to_string(trajectory_->getNextOrdinal());
            std::vector<bool> waterMask;
            vegPanel_->getService().captureScenarioState(
                *trajectory_,
                res->classification,
                waterMask,
                meta
            );
        } else {
            ImGui::OpenPopup("CaptureFailed");
        }

        // Auto-select and cache analysis for the new state
        if (!trajectory_->getTimeSlices().empty()) {
            selectedSliceIndex_ = (int)trajectory_->getTimeSlices().size() - 1;
            applyGhostVisualization(trajectory_->getTimeSlices().back());
        }
    }

    // Project Persistence
    ImGui::Spacing();
    ImGui::InputText("Nome do Projeto", projectRootName_, sizeof(projectRootName_));
    if (ImGui::Button("Save Project", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
        std::string scenariosPath = std::string(projectRootName_) + ".strata";
        std::string trajPath = std::string(projectRootName_) + "_traj.strata";
        vegPanel_->saveScenarios(scenariosPath);
        Application::Services::FourthDimensionService::saveTrajectory(*trajectory_, ".", trajPath);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Project", ImVec2(-1, 0))) {
        std::string scenariosPath = std::string(projectRootName_) + ".strata";
        std::string trajPath = std::string(projectRootName_) + "_traj.strata";
        
        vegPanel_->loadScenarios(scenariosPath);
        Application::Services::FourthDimensionService::loadTrajectory(*trajectory_, ".", trajPath);

        // Fix: Automatically visualize the last state to ensure patch cache is built
        if (!trajectory_->getTimeSlices().empty()) {
            selectedSliceIndex_ = (int)trajectory_->getTimeSlices().size() - 1;
            applyGhostVisualization(trajectory_->getTimeSlices().back());
            ghostMode_ = true; // Enter ghost mode to show the loaded data
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
            Application::Services::World3DService::resetVisualization();
        }
        ImGui::Separator();
    }

    // Simulation Controls (Moved here per user request)
    ImGui::Separator();
    ImGui::Text("Simulation Tools");
    if (ImGui::Button("Simulate: Stability")) {
        session_->simulateCondition(Application::Session::SimulationType::Stability);
        // Auto-select last
        if (!trajectory_->getTimeSlices().empty()) {
            selectedSliceIndex_ = (int)trajectory_->getTimeSlices().size() - 1;
            applyGhostVisualization(trajectory_->getTimeSlices().back());
            ghostMode_ = true; 
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Simulate: Fragmentation")) {
        session_->simulateCondition(Application::Session::SimulationType::Fragmentation);
        if (!trajectory_->getTimeSlices().empty()) {
            selectedSliceIndex_ = (int)trajectory_->getTimeSlices().size() - 1;
            applyGhostVisualization(trajectory_->getTimeSlices().back());
            ghostMode_ = true; 
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Simulate: Deforestation")) {
        session_->simulateCondition(Application::Session::SimulationType::Deforestation);
        if (!trajectory_->getTimeSlices().empty()) {
            selectedSliceIndex_ = (int)trajectory_->getTimeSlices().size() - 1;
            applyGhostVisualization(trajectory_->getTimeSlices().back());
            ghostMode_ = true; 
        }
    }
    ImGui::Separator();

    // Trajectory List
    ImGui::Text("Trajectory (%zu states)", trajectory_->getTimeSlices().size());
    
    ImGui::BeginChild("TrajectoryList", ImVec2(0, 150), true);
    auto& slices = trajectory_->getTimeSlices();
    
    for (int i = 0; i < (int)slices.size(); ++i) {
        const auto& slice = slices[i];
        std::string label = "T" + std::to_string(slice.getOrdinalIndex()) + ": " + slice.getMetadata();
        
        ImGui::PushID(i);
        // Removal Button
        if (ImGui::Button("x", ImVec2(20, 20))) {
            trajectory_->removeTimeSlice(slice.getOrdinalIndex());
            if (selectedSliceIndex_ == i) selectedSliceIndex_ = -1;
            ImGui::PopID();
            break; // Loop must break as vector was modified
        }
        ImGui::SameLine();

        bool isSelected = (selectedSliceIndex_ == i);
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            selectedSliceIndex_ = i;
        }
        ImGui::PopID();
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
                    const auto& hydro = Application::Services::World3DService::getHydroGrid();
                    auto result = Application::Services::FourthDimensionService::computeCoherenceMean(sliceA, sliceB, hydro);
                    if (!result.ok) {
                        coherenceStatus_ = result.error.empty() ? "Unable to compute map." : result.error;
                    } else {
                        lastCoherenceMean_ = result.mean;
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
            if (ImGui::Button("Interpretar Transição A-B", ImVec2(-1, 0)) && !aiRequestPending_) {
                aiRequestPending_ = true;
                
                const auto& sA = slices[compareSliceA_];
                const auto& sB = slices[compareSliceB_];
                
                Application::DTO::Cognitive::ContextBundleDTO bundle;
                bundle.bundleId = "TRANSITION-" + sA.getMetadata() + "-" + sB.getMetadata();
                bundle.intent = "transition_analysis";
                bundle.trajectorySummary = "SSI Map Mean: " + std::to_string(lastCoherenceMean_) + "\n" +
                                         "State A: " + getClassDistribution(sA) + "\n" +
                                         "State B: " + getClassDistribution(sB);

                session_->requestAIInterpretation(bundle, 
                    Application::Services::Cognitive::InterpretationMode::CoherenceCheck,
                    [this](const auto& snapshot) {
                        std::lock_guard<std::mutex> lock(insightMutex_);
                        stagedAiSnapshot_ = snapshot;
                        showAiModal_ = true;
                        aiRequestPending_ = false;
                        aiResultReady_ = true; // Signal main thread
                    });
            }
        } else {
            ImGui::TextDisabled("Calcule o mapa de coerência para habilitar.");
        }

        if (hermeneuticInProgress_) ImGui::TextDisabled("Analisando transição...");
        if (!hermeneuticInsight_.empty()) {
            ImGui::SliderFloat("Altura do Insight", &insightWindowHeight_, 100.0f, 600.0f);
            ImGui::BeginChild("HermeneuticOutput", ImVec2(0, insightWindowHeight_), true);
            ImGui::TextWrapped("%s", hermeneuticInsight_.c_str());
            ImGui::EndChild();
            if (ImGui::TreeNode("Dados de Origem (Contexto Compartilhado com LLM)")) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 1.0f, 1.0f));
                ImGui::TextWrapped("Prompt Completo / Contexto Selecionado:");
                ImGui::Separator();
                ImGui::TextWrapped("%s", hermeneuticContext_.c_str());
                ImGui::PopStyleColor();
                ImGui::TreePop();
            }
            if (ImGui::Button("Salvar Análise de Transição")) saveAnalysisToFile(hermeneuticInsight_, "transition");
        }

        // --- SEÇÃO 2: Análise de Trajetória Multi-Estado ---
        ImGui::Separator();
        ImGui::Text("Análise de Trajetória (DDD v1.1)");
        
        // Patch Discovery Logic
        if (selectedSliceIndex_ >= 0) {
            const auto& slice = slices[selectedSliceIndex_];
            if (!slice.getEcologicalCoverState().empty() && !slice.isProxy()) {
                 // Calculate if not cached (simple check, ideally we should cache properly)
                 // But we have lastPatchAnalysis_ which is updated in applyGhostVisualization.
                 // So we can use it!
            }
            ImGui::Text("Patches detectados no estado selecionado: %d", lastPatchCount_);
            
            if (ImGui::Button("Estatísticas da Paisagem")) {
                ImGui::OpenPopup("LandscapeStatsPopup");
            }
            
            if (ImGui::BeginPopup("LandscapeStatsPopup")) {
                const auto& s = lastPatchAnalysis_.summary;
                if (s.patchCount > 0) {
                    ImGui::Text("Total Patches: %d", s.patchCount);
                    ImGui::Separator();
                    ImGui::Text("Área (ha):");
                    ImGui::BulletText("Total: %.2f", s.areaTotal / 10000.0);
                    ImGui::BulletText("Média: %.2f +/- %.2f", s.areaMean / 10000.0, s.areaStdDev / 10000.0);
                    ImGui::BulletText("Min/Max: %.2f / %.2f", s.areaMin / 10000.0, s.areaMax / 10000.0);
                    ImGui::Separator();
                    ImGui::Text("Shape Index:");
                    ImGui::BulletText("Média: %.2f +/- %.2f", s.meanShapeIndex, s.shapeIndexStdDev);
                    ImGui::BulletText("Min/Max: %.2f / %.2f", s.shapeIndexMin, s.shapeIndexMax);
                } else {
                    ImGui::Text("Nenhuma análise disponível para este estado.");
                }
                ImGui::EndPopup();
            }
        }

        if (ImGui::InputInt("ID do Patch", &selectedPatchId_)) {
            if (selectedPatchId_ > 0 && !lastPatchAnalysis_.labelImage.labels.empty()) {
                Application::Services::World3DService::highlightPatch(lastPatchAnalysis_.labelImage.labels, selectedPatchId_);
            }
        }
        if (selectedPatchId_ > 0) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[Escolhido: %d]", selectedPatchId_);
            
            // Show metrics if available in cache
            if (selectedPatchId_ <= (int)lastPatchAnalysis_.patches.size()) {
                const auto& m = lastPatchAnalysis_.patches[selectedPatchId_ - 1];
                ImGui::BulletText("Área: %.2f ha | SI: %.2f", m.area / 10000.0, m.shape_index);
            }
        }
        
        ImGui::BeginDisabled(aiRequestPending_);
        if (ImGui::Button("Analisar Trajetória do Patch", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
            if (selectedPatchId_ < 0) {
                llmErrorMessage_ = "ID do patch inválido.";
            } else {
                aiRequestPending_ = true;
                
                using namespace Core::Domain::FourthDimension::PatchTrajectory;
                PatchTrajectory pt(selectedPatchId_);
                
                // ... (Keep historical aggregation logic as is, it's correct for summary generation) ...
                auto& slicesRef = trajectory_->getTimeSlices();
                double refX = -1.0, refY = -1.0;
                for (size_t i = 0; i < slicesRef.size(); ++i) {
                    auto& slice = slicesRef[i];
                    if (slice.isProxy()) Application::Services::FourthDimensionService::loadSliceFromDisk(slice);
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
                        ps.adjacencyByClass[1] = 50.0f;
                        pt.addState(ps);
                    }
                }

                auto nameResolver = [this](int code) -> std::string {
                    if (vegPanel_) {
                        const auto& scenarios = vegPanel_->getScenarioDTOs();
                        if (code >= 0 && code < (int)scenarios.size()) return "Scenario: " + scenarios[code].id;
                        if (code >= 0 && code <= 2) return Core::Domain::Vegetation::VegetationType(static_cast<Core::Domain::Vegetation::VegetationCode>(code)).toString();
                    }
                    return "Classe " + std::to_string(code);
                };

                std::string summary = Application::Services::FourthDimensionService::generatePatchTrajectorySummary(pt, nameResolver);
                
                Application::DTO::Cognitive::ContextBundleDTO bundle;
                bundle.bundleId = "PATCH-TRAJECTORY-" + std::to_string(selectedPatchId_);
                bundle.trajectorySummary = summary;

                session_->requestAIInterpretation(bundle, 
                    Application::Services::Cognitive::InterpretationMode::TrajectoryReading,
                    [this](const auto& snapshot) {
                        std::lock_guard<std::mutex> lock(insightMutex_);
                        stagedAiSnapshot_ = snapshot;
                        showAiModal_ = true;
                        aiRequestPending_ = false;
                        aiResultReady_ = true; // Signal main thread
                    });
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Análise Tática Global", ImVec2(-1, 0))) {
            aiRequestPending_ = true;
            
            std::string context = "Slices History:\n";
            for (size_t i = 0; i < slices.size(); ++i) {
                context += "T" + std::to_string(i) + ": " + getClassDistribution(slices[i]) + "\n";
            }
            
            Application::DTO::Cognitive::ContextBundleDTO bundle;
            bundle.bundleId = "GLOBAL-TACTICAL-ANALYSIS";
            bundle.trajectorySummary = context;

            session_->requestAIInterpretation(bundle, 
                Application::Services::Cognitive::InterpretationMode::TrajectoryReading,
                [this](const auto& snapshot) {
                    std::lock_guard<std::mutex> lock(insightMutex_);
                    stagedAiSnapshot_ = snapshot;
                    showAiModal_ = true;
                    aiRequestPending_ = false;
                    aiResultReady_ = true; // Signal main thread
                });
        }
        ImGui::EndDisabled();

        if (aiRequestPending_) ImGui::TextDisabled("Qwen está analisando trajetórias...");
        
        // Modal Rendering
        UI::Components::InterpretationModal::Draw("AI Cognitive Interpretation", showAiModal_, lastAiSnapshot_, [this](const auto& snap) {
            session_->saveInterpretationSnapshotDTO(snap);
        });

        
        if (!llmErrorMessage_.empty()) ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[Erro] %s", llmErrorMessage_.c_str());

    } else {
        ImGui::TextDisabled("Serviço de IA não disponível.");
    }

    ImGui::End();
}

} // namespace UI::Panels
