#pragma once

#include <cstdint>
#include <string>
#include "core/value_objects/Resolution.hpp"

namespace Core::Domain {

using WorldID = uint32_t;

class World {
public:
    World(WorldID id, const std::string& name, const ValueObjects::Resolution& resolution)
        : id_(id), name_(name), resolution_(resolution) {}

    [[nodiscard]] WorldID getId() const { return id_; }
    [[nodiscard]] const std::string& getName() const { return name_; }
    [[nodiscard]] const ValueObjects::Resolution& getResolution() const { return resolution_; }

private:
    WorldID id_;
    std::string name_;
    ValueObjects::Resolution resolution_;
};

} // namespace Core::Domain
