#include "TerritorialHypothesisPanel.hpp"
#include "imgui.h"
#include "core/domain/land_use/LandUsePotential.hpp"
// AllocationRule is in TerritorialHypothesis.hpp
#include "world3d/World3D.hpp"
#include "core/domain/resilience/ResilienceAnalyser.hpp" // New
#include <glm/glm.hpp>
#include <cmath>
#include <cstring>

namespace UI::Panels {

using namespace Core::Domain::LandUse;
using namespace Core::Domain::Territory;

TerritorialHypothesisPanel::TerritorialHypothesisPanel() 
    : currentHypothesis_("h-default", "Default Hypothesis", HypothesisType::Exploratory)
{
    // Pre-populate with Standard Types
    // Pre-populate with Standard Types
    currentHypothesis_.addLandUseType(LandUsePotential("campestre", "Campestre", glm::vec3(0.6f, 0.8f, 0.2f))); // Yellow-Green
    currentHypothesis_.addLandUseType(LandUsePotential("florestal_nat", "Florestal Natural", glm::vec3(0.0f, 0.5f, 0.0f))); // Dark Green
    currentHypothesis_.addLandUseType(LandUsePotential("florestal_cult", "Florestal Cultivado", glm::vec3(0.0f, 0.8f, 0.4f))); // Medium Green
    currentHypothesis_.addLandUseType(LandUsePotential("antropico", "Antrópico/Urbano", glm::vec3(0.5f, 0.5f, 0.5f))); // Grey
    currentHypothesis_.addLandUseType(LandUsePotential("agua", "Água", glm::vec3(0.0f, 0.0f, 0.8f))); // Blue
}

void TerritorialHypothesisPanel::draw(bool* open) {
    if (!open || !(*open)) return;

    checkAsyncCompletion();

    if (ImGui::Begin("Territorial Hypothesis Inspector", open)) {
        
        ImGui::TextDisabled("ID: %s", currentHypothesis_.getId().c_str());
        
        // Editable Name
        ImGui::SameLine();
        ImGui::Text("| Name: %s", currentHypothesis_.getName().c_str());

        // Hypothesis Type Selector
        const char* types[] = { "Exploratory", "Scenario", "ConstraintTest" };
        if (ImGui::Combo("Type", &selectedHypothesisType_, types, IM_ARRAYSIZE(types))) {
            // Update model
        }

        ImGui::Separator();
        
        if (ImGui::BeginTabBar("HypothesisTabs")) {
            if (ImGui::BeginTabItem("Land Use Types")) {
                drawLandUseEditor();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Allocation Rules")) {
                drawAllocationRules();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Evaluation")) {
                drawEvaluationSection();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Resilience")) {
                drawResilienceSection();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

    }
    ImGui::End();
}

void TerritorialHypothesisPanel::drawLandUseEditor() {
    ImGui::Spacing();
    ImGui::Text("Defined Land Use Potentials:");
    
    // Listing
    const auto& uses = currentHypothesis_.getLandUseTypes(); 
    bool openPopup = false;

    if (ImGui::BeginTable("LandUseTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();

        for (const auto& use : uses) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(use.getId().c_str());
            
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(use.getName().c_str());
            
            ImGui::TableSetColumnIndex(2);
            ImGui::PushID(use.getId().c_str());
            if (ImGui::Button("Edit")) {
                editId_ = use.getId();
                editName_ = use.getName();
                const auto& c = use.getColor();
                editColor_[0] = c.r; editColor_[1] = c.g; editColor_[2] = c.b;
                showEditModal_ = true;
                openPopup = true;
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    if (openPopup) {
        ImGui::OpenPopup("Edit Land Use");
    }

    // Modal
    if (ImGui::BeginPopupModal("Edit Land Use", &showEditModal_, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Editing ID: %s", editId_.c_str());
        
        static char nameBuf[256];
        if (ImGui::IsWindowAppearing()) {
             strncpy(nameBuf, editName_.c_str(), sizeof(nameBuf));
             nameBuf[sizeof(nameBuf)-1] = 0;
        }
        
        if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
            editName_ = std::string(nameBuf);
        }
        
        ImGui::ColorEdit3("Color", editColor_);
        
        ImGui::Separator();
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            glm::vec3 newColor(editColor_[0], editColor_[1], editColor_[2]);
            // Preserve description
            std::string desc = ""; 
            for(const auto& u : currentHypothesis_.getLandUseTypes()) if(u.getId() == editId_) desc = u.getDescription();

            currentHypothesis_.updateLandUseType(LandUsePotential(editId_, editName_, newColor, desc));
            ImGui::CloseCurrentPopup();
            showEditModal_ = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            showEditModal_ = false;
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Add New Type:");
    ImGui::Button("Add Land Use (Mock)");
}

void TerritorialHypothesisPanel::drawAllocationRules() {
    ImGui::Text("Allocation Rules define WHERE things happen.");
    
    auto rules = currentHypothesis_.getAllocationRules();
    
    if (rules.empty()) {
        ImGui::TextColored(ImVec4(1,1,0,1), "No rules defined.");
    }

    for (size_t i = 0; i < rules.size(); ++i) {
        const auto& rule = rules[i];
        ImGui::PushID((int)i);
        if (ImGui::Button("X")) {
            currentHypothesis_.removeAllocationRule(i);
            ImGui::PopID();
            break; // Stop iterating to avoid invalid iterator/index
        }
        ImGui::SameLine();
        ImGui::BulletText("Rule for: %s (Priority: %d)", rule.landUseId.c_str(), rule.priority);
        ImGui::Indent();
        for (const auto& p : rule.parameters) {
             ImGui::Text("%s: %s", p.first.c_str(), p.second.c_str());
        }
        ImGui::Unindent();
        ImGui::PopID();
    }
    
    ImGui::Separator();
    ImGui::Text("Create New Rule:");
    
    // Land Use Selector
    const auto& uses = currentHypothesis_.getLandUseTypes();
    if (newRuleLandUseIndex_ >= uses.size()) newRuleLandUseIndex_ = 0;
    
    if (ImGui::BeginCombo("Land Use", uses.empty() ? "None" : uses[newRuleLandUseIndex_].getName().c_str())) {
        for (int i = 0; i < uses.size(); i++) {
            const bool is_selected = (newRuleLandUseIndex_ == i);
            if (ImGui::Selectable(uses[i].getName().c_str(), is_selected)) {
                newRuleLandUseIndex_ = i;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    
    ImGui::DragFloat("Slope Min (deg)", &newRuleSlopeMin_, 1.0f, 0.0f, 90.0f);
    ImGui::DragFloat("Slope Max (deg)", &newRuleSlopeMax_, 1.0f, 0.0f, 90.0f);
    
    // Soil Order Selector
    auto orders = Core::Domain::Soils::SiBCSHelper::getAllOrders();
    std::vector<std::string> orderNames;
    orderNames.push_back("Qualquer"); // Option to ignore soil filter
    for (auto o : orders) orderNames.push_back(Core::Domain::Soils::SiBCSHelper::getBaseName(o));
    
    if (newRuleSoilOrderIndex_ >= orderNames.size()) newRuleSoilOrderIndex_ = 0;
    
    if (ImGui::BeginCombo("Soil Order", orderNames[newRuleSoilOrderIndex_].c_str())) {
        for (int i = 0; i < orderNames.size(); i++) {
            const bool is_selected = (newRuleSoilOrderIndex_ == i);
            if (ImGui::Selectable(orderNames[i].c_str(), is_selected)) {
                newRuleSoilOrderIndex_ = i;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::DragFloat("Min Patch Size (cells)", &newRuleMinPatchSize_, 1.0f, 1.0f, 1000.0f);

    ImGui::InputInt("Priority (1=High)", &newRulePriority_);
    
    if (ImGui::Button("Add Rule")) {
        if (!uses.empty()) {
            AllocationRule newRule;
            newRule.landUseId = uses[newRuleLandUseIndex_].getId();
            newRule.priority = newRulePriority_;
            newRule.parameters["slope_min"] = std::to_string(newRuleSlopeMin_);
            newRule.parameters["slope_max"] = std::to_string(newRuleSlopeMax_);
            newRule.parameters["soil_order"] = orderNames[newRuleSoilOrderIndex_];
            newRule.parameters["min_patch_size"] = std::to_string(newRuleMinPatchSize_);
            currentHypothesis_.addAllocationRule(newRule);
        }
    }
}

void TerritorialHypothesisPanel::drawEvaluationSection() {
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::TextWrapped("Click 'Evaluate' to run the Territorial Coherence Service against the current Territory.");
    
    // Data Integrity Check
    size_t vertexCount = World3D::getVertices().size();
    size_t soilCount = World3D::getSoilClasses().size();
    
    // Display Statistics (calculated from last update)
    ImGui::Text("Terrain Analysis:");
    ImGui::BulletText("Vertices: %zu", vertexCount);
    ImGui::BulletText("Slope Range: %.1f deg to %.1f deg (Avg: %.1f)", minSlope_, maxSlope_, avgSlope_);

    bool disabled = isAnalyzing_ || (vertexCount == 0);

    if (vertexCount == 0) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "No Terrain Loaded.");
    } else if (vertexCount != soilCount) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Warning: Soil Data Mismatch (Terrain: %zu, Soil: %zu).", vertexCount, soilCount);
        ImGui::TextWrapped("Please run the Soil Simulation (Simulador de Solos) to synchronize data before evaluating.");
        // We still allow evaluation, but expect it to fail/return error
    }

    if (disabled) ImGui::BeginDisabled();

    if (ImGui::Button("Evaluate Coherence", ImVec2(150, 40))) {
        // Prepare Data for Async
        analysisStatus_ = "Calculating Slopes and Evaluating...";
        isAnalyzing_ = true;
        
        // Copy Data (Main Thread)
        auto vertices = World3D::getVertices(); // Copy
        auto soils = World3D::getSoilClasses(); // Copy
        auto hypothesis = currentHypothesis_; // Copy hypothesis
        
        // Launch Async
         analysisFuture_ = std::async(std::launch::async, [vertices = std::move(vertices), soils = std::move(soils), hypothesis]() mutable -> AnalysisResult {
             // 1. Calc Slopes
             std::vector<float> localSlopes;
             localSlopes.reserve(vertices.size());
             
             double sum = 0;
             float minS = 90.0f, maxS = 0.0f;

             // Robust Geometry Calc
             for (size_t i = 0; i < vertices.size(); i += 3) {
                 float angle = 0.0f;
                 if (i + 2 < vertices.size()) {
                     glm::vec3 p0(vertices[i].pos.x, vertices[i].pos.y, vertices[i].pos.z);
                     glm::vec3 p1(vertices[i+1].pos.x, vertices[i+1].pos.y, vertices[i+1].pos.z);
                     glm::vec3 p2(vertices[i+2].pos.x, vertices[i+2].pos.y, vertices[i+2].pos.z);
                     glm::vec3 n = glm::normalize(glm::cross(p1-p0, p2-p0));
                     angle = glm::degrees(std::acos(std::clamp(std::abs(n.z), 0.0f, 1.0f)));
                 }
                 for(int k=0;k<3;k++) {
                     if (i+k < vertices.size()) {
                         localSlopes.push_back(angle);
                         if (angle < minS) minS = angle;
                         if (angle > maxS) maxS = angle;
                         sum += angle;
                     }
                 }
             }
             
             float avgS = !vertices.empty() ? (float)(sum / vertices.size()) : 0.0f;

             // 2. Evaluate
             Core::Domain::Territory::Territory tempT("temp_analysis", 0, 0);
             tempT.updateSlopeLayer(localSlopes);
             tempT.updateSoilLayer(soils);

             auto result = Core::Domain::Territory::TerritorialCoherenceService::evaluate(tempT, hypothesis);
             
             // 3. Update State
             // Generate Land Use Vector for Trajectory/Resilience
             auto landUseVector = Core::Domain::Territory::TerritorialCoherenceService::generateLandUseVector(tempT, hypothesis);
             
             return AnalysisResult{minS, maxS, avgS, std::move(tempT), result, std::move(landUseVector)};
        });
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Show Candidate Map", ImVec2(150, 40))) {
         analysisStatus_ = "Generating Candidate Map...";
         isAnalyzing_ = true;
         
         auto vertices = World3D::getVertices();
         auto soils = World3D::getSoilClasses();
         auto hypothesis = currentHypothesis_;

         analysisFuture_ = std::async(std::launch::async, [vertices = std::move(vertices), soils = std::move(soils), hypothesis]() mutable -> AnalysisResult {
             // 1. Calc Slopes & Points
             std::vector<float> localSlopes;
             std::vector<Core::ValueObjects::Vector3> points;
             localSlopes.reserve(vertices.size());
             points.reserve(vertices.size());

             double sum = 0;
             float minS = 90.0f, maxS = 0.0f;

             for (size_t i = 0; i < vertices.size(); i += 3) {
                 float angle = 0.0f;
                 if (i + 2 < vertices.size()) {
                     glm::vec3 p0(vertices[i].pos.x, vertices[i].pos.y, vertices[i].pos.z);
                     glm::vec3 p1(vertices[i+1].pos.x, vertices[i+1].pos.y, vertices[i+1].pos.z);
                     glm::vec3 p2(vertices[i+2].pos.x, vertices[i+2].pos.y, vertices[i+2].pos.z);
                     glm::vec3 n = glm::normalize(glm::cross(p1-p0, p2-p0));
                     angle = glm::degrees(std::acos(std::clamp(std::abs(n.z), 0.0f, 1.0f)));
                 }
                 for(int k=0;k<3;k++) {
                     if (i+k < vertices.size()) {
                         localSlopes.push_back(angle);
                         points.push_back({vertices[i+k].pos.x, vertices[i+k].pos.y, vertices[i+k].pos.z});
                         
                         if (angle < minS) minS = angle;
                         if (angle > maxS) maxS = angle;
                         sum += angle;
                     }
                 }
             }
             float avgS = !vertices.empty() ? (float)(sum / vertices.size()) : 0.0f;

             Core::Domain::Territory::Territory tempT("temp_vis", 0, 0);
             tempT.updateSlopeLayer(localSlopes);
             tempT.updateSoilLayer(soils);
             
             auto colors = Core::Domain::Territory::TerritorialCoherenceService::generateVisualizationColors(tempT, hypothesis);
             
             // Dispatch to World3D (Assumed thread-safe or queued)
             World3D::clear();
             World3D::loadPointCloud(points, colors, "Hypothesis Candidates");

             // Return Result
             Core::Domain::Shared::ValueObjects::CoherenceScore dummyScore(0.0f, "Visualization Generated");
             return AnalysisResult{minS, maxS, avgS, std::move(tempT), dummyScore};
         });
    }

    if (disabled) ImGui::EndDisabled();

    // Results Display
    ImGui::Text("Result:");
    if (!evaluationStatus_.empty()) {
        ImGui::Text("Score: %.2f", lastScore_);
        
        ImVec4 color = (evaluationStatus_ == "COHERENT") ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1);
        ImGui::TextColored(color, "Status: %s", evaluationStatus_.c_str());
        
        ImGui::TextWrapped("Description: %s", scoreDescription_.c_str());
    } else {
        ImGui::TextDisabled("No evaluation run yet.");
    }
}




void TerritorialHypothesisPanel::checkAsyncCompletion() {
    if (isAnalyzing_ && analysisFuture_.valid()) {
        auto status = analysisFuture_.wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::ready) {
            auto result = analysisFuture_.get();
            
            // Update UI State on Main Thread
            this->minSlope_ = result.minSlope;
            this->maxSlope_ = result.maxSlope;
            this->avgSlope_ = result.avgSlope;
            this->currentTerritory_ = std::move(result.territory);
            
            this->lastScore_ = result.evaluation.getValue();
            this->evaluationStatus_ = result.evaluation.isCoherent() ? "COHERENT" : "INCOHERENT";
            this->scoreDescription_ = result.evaluation.getDescription();

            // --- SPACE-TIME CONTEXT INTEGRATION ---
            // 1. Commit State to Trajectory
            // User the generated vector from async result
            this->currentTerritory_.commitState(this->currentHypothesis_.getId(), result.landUse);

            // 2. Run Resilience Analysis
            this->lastResilienceReport_ = Core::Domain::Resilience::ResilienceAnalyser::analyzeTrajectory(this->currentTerritory_.getTrajectory());
            
            isAnalyzing_ = false;
            analysisStatus_ = "Ready";
        }
    }
}

void TerritorialHypothesisPanel::drawResilienceSection() {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Resilience Analysis (Trajectory)");
    
    if (currentTerritory_.getTrajectory().size() < 2) {
        ImGui::TextDisabled("Insufficient history. Run 'Evaluate Coherence' at least twice with changes.");
        return;
    }

    // Metrics
    ImGui::Text("Trajectory Depth: %zu events", currentTerritory_.getTrajectory().size());
    
    ImGui::Separator();
    ImGui::Text("Mean Spatial Overlap (Similarity t vs t+1):");
    ImGui::ProgressBar(lastResilienceReport_.meanSpatialOverlap, ImVec2(-1, 0), 
        std::to_string(lastResilienceReport_.meanSpatialOverlap * 100).c_str());

    ImGui::Spacing();
    ImGui::Text("System State Assessment:");
    ImVec4 stateColor = lastResilienceReport_.isResilient ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1);
    if (lastResilienceReport_.meanSpatialOverlap > 0.99f) stateColor = ImVec4(1,1,0,1); // Stasis warning
    
    ImGui::TextColored(stateColor, "%s", lastResilienceReport_.finalAssessment.c_str());

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Event Log", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (const auto& log : lastResilienceReport_.eventLogs) {
             ImGui::BulletText("%s", log.c_str());
        }
    }
}

} // namespace UI::Panels
