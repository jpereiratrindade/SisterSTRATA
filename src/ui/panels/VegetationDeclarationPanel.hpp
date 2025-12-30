#pragma once

#include "core/domain/vegetation/VegetationSystemOriginal.hpp"
#include "core/domain/vegetation/VegetationDeclarationService.hpp"
#include "core/domain/vegetation/VegetationType.hpp"

namespace UI::Panels {

class VegetationDeclarationPanel {
public:
    VegetationDeclarationPanel();

    void draw(bool* open);

    // Access to the aggregate for rendering elsewhere if needed (or we render here?)
    const Core::Domain::Vegetation::VegetationSystemOriginal& getSystem() const { return system_; }

private:
    Core::Domain::Vegetation::VegetationSystemOriginal system_;
    Core::Domain::Vegetation::VegetationDeclarationService service_;

    // UI State
    char idBuffer_[64] = "Hypothesis_01";
    int selectedType_ = 0; // 0=Campestre, 1=Florestal
    float minSlope_ = 0.0f;
    float maxSlope_ = 90.0f;
    float maxDistDrainage_ = 0.0f; // 0 = ignored
};

} // namespace UI::Panels
