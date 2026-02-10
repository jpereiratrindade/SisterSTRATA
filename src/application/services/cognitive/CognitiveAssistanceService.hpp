#pragma once

#include "src/application/ports/ILLMService.hpp"
#include "src/application/dtos/cognitive/ContextBundleDTO.hpp"
#include "src/application/dtos/cognitive/InterpretationSnapshotDTO.hpp"
#include <string>
#include <vector>
#include <functional>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace Application::Services::Cognitive {

/**
 * @brief Available interpretation modes for the AI.
 */
enum class InterpretationMode {
    ThemeAnalysis,
    DiscursiveDraft,
    TrajectoryReading,
    CoherenceCheck,
    GlobalSynthesis
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

    void setLLMService(Ports::ILLMService* llmService) {
        llmService_ = llmService;
    }

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
            snapshot.createdAt = nowIsoLike();
            snapshot.intent = modeToString(mode);
            snapshot.inputContextSummary = summarizeBundleScope(bundle);
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
        prompt += "SCOPE: narratives=" + std::to_string(bundle.narratives.size())
               + " discursive=" + std::to_string(bundle.discursive.size())
               + " recommendationSnapshots=" + std::to_string(countRecommendationSnapshots(bundle.recommendation)) + "\n";
        
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

        if (!bundle.trajectoryImpactProfile.empty()) {
            prompt += "\n[TRAJECTORY IMPACT PROFILE (ANALYTICAL)]\n";
            prompt += bundle.trajectoryImpactProfile + "\n";
        }

        prompt += "\n### INSTRUCTIONS ###\n";
        switch (mode) {
            case InterpretationMode::ThemeAnalysis:
                prompt += "Based on the provided narratives, identify the dominant themes and explicit contradictions. Highlight tensions between different viewpoints.";
                break;
            case InterpretationMode::DiscursiveDraft:
                prompt += "Based on the provided Discursive Systems, propose a refined draft following the format: PROBLEM -> ACTION -> MECHANISM -> EFFECT. "
                          "Do not introduce external facts; keep the structure consistent with the provided context.";
                break;
            case InterpretationMode::TrajectoryReading:
                prompt += "Analyze the recommendation trajectory. Identify if there are major shifts in focus or persistent themes over time.";
                break;
            case InterpretationMode::CoherenceCheck:
                prompt += "Evaluate the coherence between the narratives, discursive systems, and recommendations. Are the proposed actions consistent with the observed problems?";
                break;
            case InterpretationMode::GlobalSynthesis:
                prompt += "Perform a Strategic Global Synthesis. Evaluate the alignment across all observational layers:\n"
                          "- Do the [NARRATIVE OBSERVATIONS] justify the [DISCURSIVE SYSTEMS]?\n"
                          "- Do the [RECOMMENDATION TRAJECTORY] actions logically follow from the identified mechanisms?\n"
                          "- Identify any 'blind spots' where an observation has no corresponding system/action, or vice-versa.\n"
                          "Provide a high-level strategic assessment of the project's coherence.";
                break;
        }

        return prompt;
    }

    static std::string nowIsoLike() {
        auto now = std::time(nullptr);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &now);
#else
        localtime_r(&now, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    static size_t countRecommendationSnapshots(const std::string& recommendationPacket) {
        static const std::string marker = "=> RECOMMENDATION SNAPSHOT [";
        size_t count = 0;
        size_t pos = 0;
        while ((pos = recommendationPacket.find(marker, pos)) != std::string::npos) {
            ++count;
            pos += marker.size();
        }
        return count;
    }

    static std::string summarizeBundleScope(const DTO::Cognitive::ContextBundleDTO& bundle) {
        std::ostringstream ss;
        ss << "Context Bundle: " << bundle.bundleId
           << " | narratives=" << bundle.narratives.size()
           << " | discursive=" << bundle.discursive.size()
           << " | recommendationSnapshots=" << countRecommendationSnapshots(bundle.recommendation);
        return ss.str();
    }

    std::string modeToString(InterpretationMode mode) {
        switch (mode) {
            case InterpretationMode::ThemeAnalysis: return "theme_analysis";
            case InterpretationMode::DiscursiveDraft: return "discursive_draft";
            case InterpretationMode::TrajectoryReading: return "trajectory_reading";
            case InterpretationMode::CoherenceCheck: return "coherence_check";
            case InterpretationMode::GlobalSynthesis: return "global_synthesis";
            default: return "unknown";
        }
    }

    Ports::ILLMService* llmService_;
};

} // namespace Application::Services::Cognitive
