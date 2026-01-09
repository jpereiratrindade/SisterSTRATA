#pragma once

#include <string>
#include <vector>
#include <memory>
#include "src/observational/impact_profile/domain/value_objects/ReferenceFrame.hpp"
#include "src/observational/impact_profile/domain/value_objects/StructuralDeviation.hpp"
#include "src/observational/impact_profile/domain/value_objects/TemporalDeviationPattern.hpp"

namespace SisterSTRATA::Observational::ImpactProfile::Domain {

/**
 * @brief Aggregate Root representing the analytical profile of a trajectory's impact (deformation).
 * 
 * This entity is IMMUTABLE after creation, ensuring scientific auditability.
 * It does not contain the trajectories themselves, but the analysis of their relationship.
 */
class TrajectoryImpactProfile {
public:
    using ID = std::string;

    TrajectoryImpactProfile(
        ID id,
        std::string observedTrajectoryId,
        ReferenceFrame referenceFrame,
        StructuralDeviation structuralDeviation,
        TemporalDeviationPattern temporalPattern
    ) : id_(std::move(id)),
        observedTrajectoryId_(std::move(observedTrajectoryId)),
        referenceFrame_(std::move(referenceFrame)),
        structuralDeviation_(std::move(structuralDeviation)),
        temporalPattern_(std::move(temporalPattern)) {}

    // Getters - Read-Only Access
    [[nodiscard]] const ID& getId() const { return id_; }
    [[nodiscard]] const std::string& getObservedTrajectoryId() const { return observedTrajectoryId_; }
    [[nodiscard]] const ReferenceFrame& getReference() const { return referenceFrame_; }
    [[nodiscard]] const StructuralDeviation& getStructuralDeviation() const { return structuralDeviation_; }
    [[nodiscard]] const TemporalDeviationPattern& getTemporalPattern() const { return temporalPattern_; }

private:
    ID id_;
    std::string observedTrajectoryId_;
    ReferenceFrame referenceFrame_;
    StructuralDeviation structuralDeviation_;
    TemporalDeviationPattern temporalPattern_;
};

} // namespace SisterSTRATA::Observational::ImpactProfile::Domain
