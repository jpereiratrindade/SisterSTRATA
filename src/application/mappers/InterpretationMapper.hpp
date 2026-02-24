#pragma once

#include "observational/interpretation/entities/InterpretationSnapshot.hpp"
#include "application/dtos/cognitive/InterpretationSnapshotDTO.hpp"

namespace Application::Mappers::Interpretation {

inline Application::DTO::Cognitive::InterpretationSnapshotDTO toDTO(const SisterSTRATA::Observational::Interpretation::InterpretationSnapshot& domain) {
    Application::DTO::Cognitive::InterpretationSnapshotDTO dto;
    dto.snapshotId = domain.getId();
    dto.createdAt = domain.getCreatedAt();
    dto.intent = domain.getIntent();
    dto.inputContextSummary = domain.getInputContextSummary();
    dto.aiOutput = domain.getAiOutput();
    dto.promptVersion = domain.getPromptVersion();
    dto.sourceBundleId = domain.getSourceBundleId();
    return dto;
}

inline SisterSTRATA::Observational::Interpretation::InterpretationSnapshot toDomain(const Application::DTO::Cognitive::InterpretationSnapshotDTO& dto) {
    return SisterSTRATA::Observational::Interpretation::InterpretationSnapshot(
        dto.snapshotId,
        dto.createdAt,
        dto.intent,
        dto.inputContextSummary,
        dto.aiOutput,
        dto.promptVersion,
        dto.sourceBundleId
    );
}

} // namespace Application::Mappers::Interpretation
