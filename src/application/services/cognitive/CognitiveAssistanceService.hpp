#pragma once

#include "src/application/ports/ILLMService.hpp"
#include "src/application/dtos/cognitive/ContextBundleDTO.hpp"
#include "src/application/dtos/cognitive/InterpretationSnapshotDTO.hpp"
#include <string>
#include <vector>
#include <functional>
#include <ctime>

namespace Application::Services::Cognitive {

/**
 * @brief Available interpretation modes for the AI.
 */
enum class InterpretationMode {
    ThemeAnalysis,
    DiscursiveDraft,
    TrajectoryReading,
    CoherenceCheck
};

/**
 * @brief Higher-level service that coordinates AI interpretations using ContextBundles.
 * Ensures the "epistemological memory" via InterpretationSnapshots.
 */
class CognitiveAssistanceService {
public:
    using SnapshotCallback = std::function<void(const DTO::Cognitive::InterpretationSnapshotDTO&)>;

    CognitiveAssistanceService(Ports::ILLMService* llmService) 
        : llmService_(llmService) {}

    /**
     * @brief Interprets a ContextBundle using the specified mode.
     */
    void interpret(const DTO::Cognitive::ContextBundleDTO& bundle, 
                   InterpretationMode mode, 
                   SnapshotCallback callback) {
        
        if (!llmService_) return;

        std::vector<Ports::LLMMessage> messages;
        messages.push_back({Ports::LLMRole::User, formatPrompt(bundle, mode)});

        llmService_->requestCompletion(messages, [this, bundle, mode, callback](const Ports::ILLMService::Response& res) {
            DTO::Cognitive::InterpretationSnapshotDTO snapshot;
            snapshot.snapshotId = "SNAP-" + std::to_string(std::time(nullptr));
            snapshot.intent = modeToString(mode);
            snapshot.inputContextSummary = "Context Bundle: " + bundle.bundleId;
            snapshot.aiOutput = res.success ? res.content : "Error: " + res.errorMessage;
            snapshot.promptVersion = "Canonical-Prompt-v1.1";
            snapshot.sourceBundleId = bundle.bundleId;
            
            // Note: We don't save to session here, we let the UI/Caller decide if they want to persist it
            callback(snapshot);
        });
    }

private:
    std::string formatPrompt(const DTO::Cognitive::ContextBundleDTO& bundle, InterpretationMode mode) {
        std::string prompt;
        prompt += "INTERPRETATION MODE: " + modeToString(mode) + "\n\n";
        
        prompt += "### DATA CONTEXT ###\n";
        
        if (!bundle.narratives.empty()) {
            prompt += "\n[NARRATIVE OBSERVATIONS]\n";
            for (const auto& n : bundle.narratives) prompt += n + "\n";
        }

        if (!bundle.discursive.empty()) {
            prompt += "\n[DISCURSIVE SYSTEMS]\n";
            for (const auto& d : bundle.discursive) prompt += d + "\n";
        }

        if (!bundle.recommendation.empty()) {
            prompt += "\n[RECOMMENDATION TRAJECTORY]\n";
            prompt += bundle.recommendation + "\n";
        }

        if (!bundle.trajectorySummary.empty()) {
            prompt += "\n[FOURTH DIMENSION SUMMARY]\n";
            prompt += bundle.trajectorySummary + "\n";
        }

        prompt += "\n### INSTRUCTIONS ###\n";
        switch (mode) {
            case InterpretationMode::ThemeAnalysis:
                prompt += "Based on the provided narratives, identify the dominant themes and explicit contradictions. Highlight tensions between different viewpoints.";
                break;
            case InterpretationMode::DiscursiveDraft:
                prompt += "Based on the narratives, propose a draft for a Discursive System following the format: PROBLEM -> ACTION -> MECHANISM -> EFFECT. State your reasoning clearly.";
                break;
            case InterpretationMode::TrajectoryReading:
                prompt += "Analyze the recommendation trajectory. Identify if there are major shifts in focus or persistent themes over time.";
                break;
            case InterpretationMode::CoherenceCheck:
                prompt += "Evaluate the coherence between the narratives, discursive systems, and recommendations. Are the proposed actions consistent with the observed problems?";
                break;
        }

        return prompt;
    }

    std::string modeToString(InterpretationMode mode) {
        switch (mode) {
            case InterpretationMode::ThemeAnalysis: return "theme_analysis";
            case InterpretationMode::DiscursiveDraft: return "discursive_draft";
            case InterpretationMode::TrajectoryReading: return "trajectory_reading";
            case InterpretationMode::CoherenceCheck: return "coherence_check";
            default: return "unknown";
        }
    }

    Ports::ILLMService* llmService_;
};

} // namespace Application::Services::Cognitive
