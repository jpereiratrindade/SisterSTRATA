#pragma once

#include "application/dtos/DiscursiveSystemDTO.hpp"
#include "application/dtos/RecommendationTrajectoryDTO.hpp"
#include "application/dtos/NarrativeDTOs.hpp"
#include "observational/discursive/entities/DiscursiveSystem.hpp"
#include "observational/recommendation/aggregates/RecommendationTrajectory.hpp"
#include "observational/narrative/entities/NarrativeState.hpp"
#include <algorithm>
#include <cctype>

namespace Application::Mappers {

namespace {

inline std::string normalizeToken(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return value;
}

} // namespace

namespace Discursive {

inline std::string sourceTypeToString(SisterSTRATA::Observational::Discursive::SourceReference::SourceType type) {
    using Type = SisterSTRATA::Observational::Discursive::SourceReference::SourceType;
    switch (type) {
        case Type::INTERVIEW: return "INTERVIEW";
        case Type::DOCUMENT: return "DOCUMENT";
        case Type::QUESTIONNAIRE: return "QUESTIONNAIRE";
        case Type::TECHNICAL_BULLETIN: return "TECHNICAL_BULLETIN";
        case Type::REPORT: return "REPORT";
        case Type::OTHER: return "OTHER";
    }
    return "OTHER";
}

inline SisterSTRATA::Observational::Discursive::SourceReference::SourceType sourceTypeFromString(const std::string& value) {
    const std::string token = normalizeToken(value);
    using Type = SisterSTRATA::Observational::Discursive::SourceReference::SourceType;
    if (token == "INTERVIEW") return Type::INTERVIEW;
    if (token == "DOCUMENT") return Type::DOCUMENT;
    if (token == "QUESTIONNAIRE") return Type::QUESTIONNAIRE;
    if (token == "TECHNICAL_BULLETIN") return Type::TECHNICAL_BULLETIN;
    if (token == "REPORT") return Type::REPORT;
    return Type::OTHER;
}

inline std::string temporalCategoryToString(SisterSTRATA::Observational::Discursive::TemporalContext::RelativeTiming category) {
    using Cat = SisterSTRATA::Observational::Discursive::TemporalContext::RelativeTiming;
    switch (category) {
        case Cat::ANCESTRAL: return "ANCESTRAL";
        case Cat::PAST: return "PAST";
        case Cat::RECENT_PAST: return "RECENT_PAST";
        case Cat::CONTEMPORARY: return "CONTEMPORARY";
        case Cat::FUTURE_VISION: return "FUTURE_VISION";
        case Cat::TIMELESS: return "TIMELESS";
        case Cat::INDETERMINATE: return "INDETERMINATE";
    }
    return "INDETERMINATE";
}

inline SisterSTRATA::Observational::Discursive::TemporalContext::RelativeTiming temporalCategoryFromString(const std::string& value) {
    const std::string token = normalizeToken(value);
    using Cat = SisterSTRATA::Observational::Discursive::TemporalContext::RelativeTiming;
    if (token == "ANCESTRAL") return Cat::ANCESTRAL;
    if (token == "PAST") return Cat::PAST;
    if (token == "RECENT_PAST") return Cat::RECENT_PAST;
    if (token == "CONTEMPORARY") return Cat::CONTEMPORARY;
    if (token == "FUTURE_VISION") return Cat::FUTURE_VISION;
    if (token == "TIMELESS") return Cat::TIMELESS;
    return Cat::INDETERMINATE;
}

inline Application::DTO::SourceReferenceDTO toDTO(const SisterSTRATA::Observational::Discursive::SourceReference& source) {
    return Application::DTO::SourceReferenceDTO{
        sourceTypeToString(source.getType()),
        source.getSourceId(),
        source.getProductionDate(),
        source.getAuthor()
    };
}

inline Application::DTO::TemporalContextDTO toDTO(const SisterSTRATA::Observational::Discursive::TemporalContext& temporal) {
    return Application::DTO::TemporalContextDTO{
        temporalCategoryToString(temporal.getCategory()),
        temporal.getLabel()
    };
}

inline SisterSTRATA::Observational::Discursive::SourceReference toDomain(const Application::DTO::SourceReferenceDTO& dto) {
    return SisterSTRATA::Observational::Discursive::SourceReference(
        sourceTypeFromString(dto.sourceType),
        dto.sourceId,
        dto.productionDate,
        dto.author
    );
}

inline SisterSTRATA::Observational::Discursive::TemporalContext toDomain(const Application::DTO::TemporalContextDTO& dto) {
    return SisterSTRATA::Observational::Discursive::TemporalContext(
        temporalCategoryFromString(dto.category),
        dto.label
    );
}

inline Application::DTO::DiscursiveSystemDTO toDTO(const SisterSTRATA::Observational::Discursive::DiscursiveSystem& system) {
    Application::DTO::DiscursiveSystemDTO dto;
    dto.id = system.getId();

    for (const auto& problem : system.getDeclaredProblems()) {
        dto.declaredProblems.push_back(problem.getStatement());
    }
    for (const auto& action : system.getDeclaredActions()) {
        dto.declaredActions.push_back(action.getStatement());
    }
    for (const auto& mechanism : system.getAllegedMechanisms()) {
        dto.allegedMechanisms.push_back(mechanism.getStatement());
    }
    for (const auto& effect : system.getExpectedEffects()) {
        dto.expectedEffects.push_back(effect.getStatement());
    }
    for (const auto& source : system.getSourceReferences()) {
        dto.sourceReferences.push_back(toDTO(source));
    }
    dto.temporalContext = toDTO(system.getTemporalContext());
    dto.interpretationMetadata = system.getInterpretationMetadata();

    return dto;
}

inline SisterSTRATA::Observational::Discursive::DiscursiveSystem toDomain(const Application::DTO::DiscursiveSystemDTO& dto) {
    std::vector<SisterSTRATA::Observational::Discursive::DeclaredProblem> problems;
    std::vector<SisterSTRATA::Observational::Discursive::DeclaredAction> actions;
    std::vector<SisterSTRATA::Observational::Discursive::AllegedMechanism> mechanisms;
    std::vector<SisterSTRATA::Observational::Discursive::ExpectedEffect> effects;
    std::vector<SisterSTRATA::Observational::Discursive::SourceReference> sources;

    for (const auto& entry : dto.declaredProblems) {
        problems.emplace_back(entry);
    }
    for (const auto& entry : dto.declaredActions) {
        actions.emplace_back(entry);
    }
    for (const auto& entry : dto.allegedMechanisms) {
        mechanisms.emplace_back(entry);
    }
    for (const auto& entry : dto.expectedEffects) {
        effects.emplace_back(entry);
    }
    for (const auto& entry : dto.sourceReferences) {
        sources.push_back(toDomain(entry));
    }

    return SisterSTRATA::Observational::Discursive::DiscursiveSystem(
        dto.id,
        std::move(problems),
        std::move(actions),
        std::move(mechanisms),
        std::move(effects),
        std::move(sources),
        toDomain(dto.temporalContext),
        dto.interpretationMetadata
    );
}

} // namespace Discursive

namespace Recommendation {

inline std::string sourceTypeToString(SisterSTRATA::Observational::Recommendation::SourceReference::SourceType type) {
    using Type = SisterSTRATA::Observational::Recommendation::SourceReference::SourceType;
    switch (type) {
        case Type::TECHNICAL_RECOMMENDATION: return "TECHNICAL_RECOMMENDATION";
        case Type::TECHNICAL_BULLETIN: return "TECHNICAL_BULLETIN";
        case Type::REPORT: return "REPORT";
        case Type::DOCUMENT: return "DOCUMENT";
        case Type::OTHER: return "OTHER";
    }
    return "OTHER";
}

inline SisterSTRATA::Observational::Recommendation::SourceReference::SourceType sourceTypeFromString(const std::string& value) {
    const std::string token = normalizeToken(value);
    using Type = SisterSTRATA::Observational::Recommendation::SourceReference::SourceType;
    if (token == "TECHNICAL_RECOMMENDATION") return Type::TECHNICAL_RECOMMENDATION;
    if (token == "TECHNICAL_BULLETIN") return Type::TECHNICAL_BULLETIN;
    if (token == "REPORT") return Type::REPORT;
    if (token == "DOCUMENT") return Type::DOCUMENT;
    return Type::OTHER;
}

inline std::string temporalCategoryToString(SisterSTRATA::Observational::Recommendation::TemporalContext::RelativeTiming category) {
    using Cat = SisterSTRATA::Observational::Recommendation::TemporalContext::RelativeTiming;
    switch (category) {
        case Cat::ANCESTRAL: return "ANCESTRAL";
        case Cat::PAST: return "PAST";
        case Cat::RECENT_PAST: return "RECENT_PAST";
        case Cat::CONTEMPORARY: return "CONTEMPORARY";
        case Cat::FUTURE_VISION: return "FUTURE_VISION";
        case Cat::TIMELESS: return "TIMELESS";
        case Cat::INDETERMINATE: return "INDETERMINATE";
    }
    return "INDETERMINATE";
}

inline SisterSTRATA::Observational::Recommendation::TemporalContext::RelativeTiming temporalCategoryFromString(const std::string& value) {
    const std::string token = normalizeToken(value);
    using Cat = SisterSTRATA::Observational::Recommendation::TemporalContext::RelativeTiming;
    if (token == "ANCESTRAL") return Cat::ANCESTRAL;
    if (token == "PAST") return Cat::PAST;
    if (token == "RECENT_PAST") return Cat::RECENT_PAST;
    if (token == "CONTEMPORARY") return Cat::CONTEMPORARY;
    if (token == "FUTURE_VISION") return Cat::FUTURE_VISION;
    if (token == "TIMELESS") return Cat::TIMELESS;
    return Cat::INDETERMINATE;
}

inline Application::DTO::SourceReferenceDTO toDTO(const SisterSTRATA::Observational::Recommendation::SourceReference& source) {
    return Application::DTO::SourceReferenceDTO{
        sourceTypeToString(source.getType()),
        source.getSourceId(),
        source.getProductionDate(),
        source.getAuthor()
    };
}

inline Application::DTO::TemporalContextDTO toDTO(const SisterSTRATA::Observational::Recommendation::TemporalContext& temporal) {
    return Application::DTO::TemporalContextDTO{
        temporalCategoryToString(temporal.getCategory()),
        temporal.getLabel()
    };
}

inline SisterSTRATA::Observational::Recommendation::SourceReference toDomain(const Application::DTO::SourceReferenceDTO& dto) {
    return SisterSTRATA::Observational::Recommendation::SourceReference(
        sourceTypeFromString(dto.sourceType),
        dto.sourceId,
        dto.productionDate,
        dto.author
    );
}

inline SisterSTRATA::Observational::Recommendation::TemporalContext toDomain(const Application::DTO::TemporalContextDTO& dto) {
    return SisterSTRATA::Observational::Recommendation::TemporalContext(
        temporalCategoryFromString(dto.category),
        dto.label
    );
}

inline Application::DTO::RecommendationSnapshotDTO toDTO(
    const SisterSTRATA::Observational::Recommendation::RecommendationSnapshot& snapshot
) {
    return Application::DTO::RecommendationSnapshotDTO{
        snapshot.getId(),
        snapshot.getRecommendationText(),
        snapshot.getContextConditions(),
        snapshot.getIntendedAction(),
        snapshot.getExpectedOutcome(),
        toDTO(snapshot.getSourceReference()),
        toDTO(snapshot.getTemporalContext())
    };
}

inline SisterSTRATA::Observational::Recommendation::RecommendationSnapshot toDomain(
    const Application::DTO::RecommendationSnapshotDTO& dto
) {
    return SisterSTRATA::Observational::Recommendation::RecommendationSnapshot(
        dto.id,
        dto.recommendationText,
        dto.contextConditions,
        dto.intendedAction,
        dto.expectedOutcome,
        toDomain(dto.sourceReference),
        toDomain(dto.temporalContext)
    );
}

inline Application::DTO::RecommendationTrajectoryDTO toDTO(
    const SisterSTRATA::Observational::Recommendation::RecommendationTrajectory& trajectory
) {
    Application::DTO::RecommendationTrajectoryDTO dto;
    dto.id = trajectory.getId();
    dto.metadata = trajectory.getMetadata();
    for (const auto& snapshot : trajectory.getSnapshots()) {
        dto.snapshots.push_back(toDTO(snapshot));
    }
    return dto;
}

inline SisterSTRATA::Observational::Recommendation::RecommendationTrajectory toDomain(
    const Application::DTO::RecommendationTrajectoryDTO& dto
) {
    SisterSTRATA::Observational::Recommendation::RecommendationTrajectory trajectory(dto.id, dto.metadata);
    for (const auto& snapshot : dto.snapshots) {
        trajectory.addSnapshot(toDomain(snapshot));
    }
    return trajectory;
}

} // namespace Recommendation

namespace Narrative {

inline std::string sourceTypeToString(SisterSTRATA::Observational::Narrative::SourceReference::SourceType type) {
    using Type = SisterSTRATA::Observational::Narrative::SourceReference::SourceType;
    switch (type) {
        case Type::INTERVIEW: return "INTERVIEW";
        case Type::TECHNICAL_REPORT: return "TECHNICAL_REPORT";
        case Type::HISTORICAL_RECORD: return "HISTORICAL_RECORD";
        case Type::SCIENTIFIC_ARTICLE: return "SCIENTIFIC_ARTICLE";
        case Type::INSTITUTIONAL_DOCUMENT: return "INSTITUTIONAL_DOCUMENT";
        case Type::MEDIA_ARTICLE: return "MEDIA_ARTICLE";
        case Type::FIELD_NOTE: return "FIELD_NOTE";
        case Type::OTHER: return "OTHER";
    }
    return "OTHER";
}

inline SisterSTRATA::Observational::Narrative::SourceReference::SourceType sourceTypeFromString(const std::string& value) {
    const std::string token = normalizeToken(value);
    using Type = SisterSTRATA::Observational::Narrative::SourceReference::SourceType;
    if (token == "INTERVIEW") return Type::INTERVIEW;
    if (token == "TECHNICAL_REPORT") return Type::TECHNICAL_REPORT;
    if (token == "HISTORICAL_RECORD") return Type::HISTORICAL_RECORD;
    if (token == "SCIENTIFIC_ARTICLE") return Type::SCIENTIFIC_ARTICLE;
    if (token == "INSTITUTIONAL_DOCUMENT") return Type::INSTITUTIONAL_DOCUMENT;
    if (token == "MEDIA_ARTICLE") return Type::MEDIA_ARTICLE;
    if (token == "FIELD_NOTE") return Type::FIELD_NOTE;
    return Type::OTHER;
}

inline std::string temporalCategoryToString(SisterSTRATA::Observational::Narrative::TemporalContext::RelativeTiming category) {
    using Cat = SisterSTRATA::Observational::Narrative::TemporalContext::RelativeTiming;
    switch (category) {
        case Cat::ANCESTRAL: return "ANCESTRAL";
        case Cat::PAST: return "PAST";
        case Cat::RECENT_PAST: return "RECENT_PAST";
        case Cat::CONTEMPORARY: return "CONTEMPORARY";
        case Cat::FUTURE_VISION: return "FUTURE_VISION";
        case Cat::TIMELESS: return "TIMELESS";
        case Cat::INDETERMINATE: return "INDETERMINATE";
    }
    return "INDETERMINATE";
}

inline SisterSTRATA::Observational::Narrative::TemporalContext::RelativeTiming temporalCategoryFromString(const std::string& value) {
    const std::string token = normalizeToken(value);
    using Cat = SisterSTRATA::Observational::Narrative::TemporalContext::RelativeTiming;
    if (token == "ANCESTRAL") return Cat::ANCESTRAL;
    if (token == "PAST") return Cat::PAST;
    if (token == "RECENT_PAST") return Cat::RECENT_PAST;
    if (token == "CONTEMPORARY") return Cat::CONTEMPORARY;
    if (token == "FUTURE_VISION") return Cat::FUTURE_VISION;
    if (token == "TIMELESS") return Cat::TIMELESS;
    return Cat::INDETERMINATE;
}

inline std::string intentToString(SisterSTRATA::Observational::Narrative::ObservationIntent::IntentType intent) {
    using Intent = SisterSTRATA::Observational::Narrative::ObservationIntent::IntentType;
    switch (intent) {
        case Intent::DESCRIPTIVE_RECORD: return "DESCRIPTIVE_RECORD";
        case Intent::EXPLORATORY_HYPOTHESIS: return "EXPLORATORY_HYPOTHESIS";
        case Intent::CONTEXTUALIZATION: return "CONTEXTUALIZATION";
        case Intent::METHODOLOGICAL_NOTE: return "METHODOLOGICAL_NOTE";
    }
    return "DESCRIPTIVE_RECORD";
}

inline SisterSTRATA::Observational::Narrative::ObservationIntent::IntentType intentFromString(const std::string& value) {
    const std::string token = normalizeToken(value);
    using Intent = SisterSTRATA::Observational::Narrative::ObservationIntent::IntentType;
    if (token == "EXPLORATORY_HYPOTHESIS") return Intent::EXPLORATORY_HYPOTHESIS;
    if (token == "CONTEXTUALIZATION") return Intent::CONTEXTUALIZATION;
    if (token == "METHODOLOGICAL_NOTE") return Intent::METHODOLOGICAL_NOTE;
    return Intent::DESCRIPTIVE_RECORD;
}

inline std::string abstractionLevelToString(SisterSTRATA::Observational::Narrative::SemanticAxis::AbstractionLevel level) {
    using Level = SisterSTRATA::Observational::Narrative::SemanticAxis::AbstractionLevel;
    switch (level) {
        case Level::LOCAL: return "LOCAL";
        case Level::REGIONAL: return "REGIONAL";
        case Level::INSTITUTIONAL: return "INSTITUTIONAL";
        case Level::GLOBAL: return "GLOBAL";
    }
    return "LOCAL";
}

inline SisterSTRATA::Observational::Narrative::SemanticAxis::AbstractionLevel abstractionLevelFromString(const std::string& value) {
    const std::string token = normalizeToken(value);
    using Level = SisterSTRATA::Observational::Narrative::SemanticAxis::AbstractionLevel;
    if (token == "REGIONAL") return Level::REGIONAL;
    if (token == "INSTITUTIONAL") return Level::INSTITUTIONAL;
    if (token == "GLOBAL") return Level::GLOBAL;
    return Level::LOCAL;
}

inline std::string spatialScopeTypeToString(SisterSTRATA::Observational::Narrative::SpatialScope::ScopeType type) {
    using Type = SisterSTRATA::Observational::Narrative::SpatialScope::ScopeType;
    switch (type) {
        case Type::NONE: return "NONE";
        case Type::POINT: return "POINT";
        case Type::PATCH_ID: return "PATCH_ID";
        case Type::REGION_BOX: return "REGION_BOX";
    }
    return "NONE";
}

inline SisterSTRATA::Observational::Narrative::SpatialScope::ScopeType spatialScopeTypeFromString(const std::string& value) {
    const std::string token = normalizeToken(value);
    using Type = SisterSTRATA::Observational::Narrative::SpatialScope::ScopeType;
    if (token == "POINT") return Type::POINT;
    if (token == "PATCH_ID") return Type::PATCH_ID;
    if (token == "REGION_BOX") return Type::REGION_BOX;
    return Type::NONE;
}

inline Application::DTO::SourceReferenceDTO toDTO(const SisterSTRATA::Observational::Narrative::SourceReference& source) {
    return Application::DTO::SourceReferenceDTO{
        sourceTypeToString(source.getType()),
        source.getSourceId(),
        source.getProductionDate(),
        source.getAuthor()
    };
}

inline Application::DTO::TemporalContextDTO toDTO(const SisterSTRATA::Observational::Narrative::TemporalContext& temporal) {
    return Application::DTO::TemporalContextDTO{
        temporalCategoryToString(temporal.getCategory()),
        temporal.getLabel()
    };
}

inline Application::DTO::ObservationIntentDTO toDTO(const SisterSTRATA::Observational::Narrative::ObservationIntent& intent) {
    return Application::DTO::ObservationIntentDTO{intentToString(intent.getType())};
}

inline Application::DTO::SemanticAxisDTO toDTO(const SisterSTRATA::Observational::Narrative::SemanticAxis& axis) {
    return Application::DTO::SemanticAxisDTO{
        axis.getLabel(),
        axis.getDescription(),
        abstractionLevelToString(axis.getLevel())
    };
}

inline Application::DTO::SpatialScopeDTO toDTO(const SisterSTRATA::Observational::Narrative::SpatialScope& scope) {
    Application::DTO::SpatialScopeDTO dto;
    dto.type = spatialScopeTypeToString(scope.getType());
    if (scope.getPatchId().has_value()) {
        dto.patchId = scope.getPatchId();
    }
    if (scope.getCoordinates().has_value()) {
        const auto coords = scope.getCoordinates().value();
        dto.coordinates = Application::DTO::SpatialCoordinatesDTO{coords.x, coords.y, coords.z};
    }
    return dto;
}

inline SisterSTRATA::Observational::Narrative::SourceReference toDomain(const Application::DTO::SourceReferenceDTO& dto) {
    return SisterSTRATA::Observational::Narrative::SourceReference(
        sourceTypeFromString(dto.sourceType),
        dto.sourceId,
        dto.productionDate,
        dto.author
    );
}

inline SisterSTRATA::Observational::Narrative::TemporalContext toDomain(const Application::DTO::TemporalContextDTO& dto) {
    return SisterSTRATA::Observational::Narrative::TemporalContext(
        temporalCategoryFromString(dto.category),
        dto.label
    );
}

inline SisterSTRATA::Observational::Narrative::ObservationIntent toDomain(const Application::DTO::ObservationIntentDTO& dto) {
    return SisterSTRATA::Observational::Narrative::ObservationIntent(
        intentFromString(dto.intentType)
    );
}

inline SisterSTRATA::Observational::Narrative::SemanticAxis toDomain(const Application::DTO::SemanticAxisDTO& dto) {
    return SisterSTRATA::Observational::Narrative::SemanticAxis(
        dto.label,
        dto.description,
        abstractionLevelFromString(dto.abstractionLevel)
    );
}

inline SisterSTRATA::Observational::Narrative::SpatialScope toDomain(const Application::DTO::SpatialScopeDTO& dto) {
    const auto type = spatialScopeTypeFromString(dto.type);
    if (type == SisterSTRATA::Observational::Narrative::SpatialScope::ScopeType::PATCH_ID && dto.patchId.has_value()) {
        return SisterSTRATA::Observational::Narrative::SpatialScope(dto.patchId.value());
    }
    if (type == SisterSTRATA::Observational::Narrative::SpatialScope::ScopeType::POINT && dto.coordinates.has_value()) {
        auto& coords = dto.coordinates.value();
        return SisterSTRATA::Observational::Narrative::SpatialScope(coords.x, coords.y, coords.z);
    }
    return SisterSTRATA::Observational::Narrative::SpatialScope();
}

inline Application::DTO::NarrativeStateDTO toDTO(const SisterSTRATA::Observational::Narrative::NarrativeState& state) {
    Application::DTO::NarrativeStateDTO dto;
    dto.id = state.getId();
    dto.source = toDTO(state.getSource());
    dto.temporalContext = toDTO(state.getTemporalContext());
    dto.intent = toDTO(state.getIntent());
    for (const auto& axis : state.getAxes()) {
        dto.axes.push_back(toDTO(axis));
    }
    dto.metadata = state.getMetadata();
    if (state.getSpatialScope().has_value()) {
        dto.spatialScope = toDTO(state.getSpatialScope().value());
    }
    return dto;
}

inline SisterSTRATA::Observational::Narrative::NarrativeState toDomain(const Application::DTO::NarrativeStateDTO& dto) {
    std::vector<SisterSTRATA::Observational::Narrative::SemanticAxis> axes;
    for (const auto& axis : dto.axes) {
        axes.push_back(toDomain(axis));
    }
    std::optional<SisterSTRATA::Observational::Narrative::SpatialScope> scope;
    if (dto.spatialScope.has_value()) {
        scope = toDomain(dto.spatialScope.value());
    }
    return SisterSTRATA::Observational::Narrative::NarrativeState(
        dto.id,
        toDomain(dto.source),
        toDomain(dto.temporalContext),
        toDomain(dto.intent),
        std::move(axes),
        dto.metadata,
        scope
    );
}

} // namespace Narrative

} // namespace Application::Mappers
