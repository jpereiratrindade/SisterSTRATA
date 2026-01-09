#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Interpretation {

using SnapshotID = std::string;

/**
 * @brief Represents a persistent AI interpretation (Epistemic Memory).
 */
class InterpretationSnapshot {
public:
    InterpretationSnapshot() = default;

    InterpretationSnapshot(
        SnapshotID id,
        std::string createdAt,
        std::string intent,
        std::string inputContextSummary,
        std::string aiOutput,
        std::string promptVersion,
        std::string sourceBundleId
    ) : m_id(std::move(id)),
        m_createdAt(std::move(createdAt)),
        m_intent(std::move(intent)),
        m_inputContextSummary(std::move(inputContextSummary)),
        m_aiOutput(std::move(aiOutput)),
        m_promptVersion(std::move(promptVersion)),
        m_sourceBundleId(std::move(sourceBundleId)) {}

    // Getters
    const SnapshotID& getId() const { return m_id; }
    const std::string& getCreatedAt() const { return m_createdAt; }
    const std::string& getIntent() const { return m_intent; }
    const std::string& getInputContextSummary() const { return m_inputContextSummary; }
    const std::string& getAiOutput() const { return m_aiOutput; }
    const std::string& getPromptVersion() const { return m_promptVersion; }
    const std::string& getSourceBundleId() const { return m_sourceBundleId; }

    // JSON Serialization
    friend void to_json(nlohmann::json& j, const InterpretationSnapshot& obj) {
        j = nlohmann::json{
            {"id", obj.m_id},
            {"createdAt", obj.m_createdAt},
            {"intent", obj.m_intent},
            {"inputContextSummary", obj.m_inputContextSummary},
            {"aiOutput", obj.m_aiOutput},
            {"promptVersion", obj.m_promptVersion},
            {"sourceBundleId", obj.m_sourceBundleId}
        };
    }

    friend void from_json(const nlohmann::json& j, InterpretationSnapshot& obj) {
        obj.m_id = j.at("id").get<SnapshotID>();
        obj.m_createdAt = j.at("createdAt").get<std::string>();
        obj.m_intent = j.at("intent").get<std::string>();
        obj.m_inputContextSummary = j.at("inputContextSummary").get<std::string>();
        obj.m_aiOutput = j.at("aiOutput").get<std::string>();
        obj.m_promptVersion = j.at("promptVersion").get<std::string>();
        obj.m_sourceBundleId = j.at("sourceBundleId").get<std::string>();
    }

private:
    SnapshotID m_id;
    std::string m_createdAt;
    std::string m_intent;
    std::string m_inputContextSummary;
    std::string m_aiOutput;
    std::string m_promptVersion;
    std::string m_sourceBundleId;
};

} // namespace SisterSTRATA::Observational::Interpretation
