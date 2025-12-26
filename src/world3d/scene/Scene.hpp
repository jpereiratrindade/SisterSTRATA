#pragma once

#include "world3d/scene/RenderObject.hpp"
#include <vector>

namespace World3D {

class Scene {
public:
    void addObject(RenderObject object) {
        objects_.push_back(object);
    }

    const std::vector<RenderObject>& getObjects() const {
        return objects_;
    }

    void clear() {
        objects_.clear();
    }

private:
    std::vector<RenderObject> objects_;
};

} // namespace World3D
