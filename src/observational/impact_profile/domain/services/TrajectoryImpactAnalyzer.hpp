#pragma once

#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "observational/impact_profile/domain/entities/TrajectoryImpactProfile.hpp"
#include "observational/impact_profile/domain/value_objects/ReferenceFrame.hpp"

namespace SisterSTRATA::Observational::ImpactProfile::Domain {

/**
 * @brief Domain Service responsible for calculating the impact profile.
 * 
 * PURE VIRTUAL INTERFACE.
 * 
 * This service encapsulates the logic of comparing an Observed Trajectory 
 * against a Reference Trajectory within a specific Context.
 * It is Stateless and side-effect free.
 */
class ITrajectoryImpactAnalyzer {
public:
    virtual ~ITrajectoryImpactAnalyzer() = default;

    /**
     * @brief Generates an Impact Profile by comparing two trajectories.
     * 
     * @param observed The trajectory effectively observed/recorded.
     * @param reference The trajectory used as a baseline (Historical/Simulated).
     * @param context The explicit reference frame context.
     * @return TrajectoryImpactProfile An immutable analysis result.
     */
    virtual TrajectoryImpactProfile analyze(
        const Core::Domain::FourthDimension::Trajectory& observed,
        const Core::Domain::FourthDimension::Trajectory& reference,
        const ReferenceFrame& context
    ) const = 0;
};

} // namespace SisterSTRATA::Observational::ImpactProfile::Domain
