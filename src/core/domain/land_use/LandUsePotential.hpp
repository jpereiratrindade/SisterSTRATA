#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Core::Domain::LandUse {

/**
 * @brief Value Object representing a potential Land Use hypothesis.
 * 
 * In STRATA, Land Use is not a state that "occupies" the land, but a hypothesis
 * defined by the user to be evaluated against the bio-physical reality.
 * It is immutable and serves as a typed classifier for coherence analysis.
 */
class LandUsePotential {
public:
    using ID = std::string;

    LandUsePotential(ID id, std::string name, glm::vec3 color = glm::vec3(0.5f), std::string description = "")
        : id_(std::move(id)), name_(std::move(name)), color_(color), description_(std::move(description)) {}

    const ID& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const glm::vec3& getColor() const { return color_; }
    const std::string& getDescription() const { return description_; }

    // Equality for VOs
    bool operator==(const LandUsePotential& other) const {
        return id_ == other.id_;
    }

private:
    ID id_;
    std::string name_;
    glm::vec3 color_;
    std::string description_;
    
    // Future: BioPhysicalConstraints constraints_;
};

} // namespace Core::Domain::LandUse
