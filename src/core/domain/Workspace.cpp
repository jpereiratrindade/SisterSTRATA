#include "core/domain/Workspace.hpp"

namespace Core::Domain {

void Workspace::addDataset(const std::string& name, const std::string& path_str) {
    ValueObjects::FilePath path(path_str);
    datasets_.push_back(std::make_unique<Dataset>(next_dataset_id_++, name, path));
}

void Workspace::createWorld(const std::string& name, uint32_t width, uint32_t height) {
    ValueObjects::Resolution res(width, height);
    if (!res.isValid()) {
        // Handle invalid resolution, throw or log? For now, we proceed or could throw.
        // Keeping it simple as per scaffold.
    }
    world_ = std::make_unique<World>(next_world_id_++, name, res);
}

const std::vector<std::unique_ptr<Dataset>>& Workspace::getDatasets() const {
    return datasets_;
}

const std::unique_ptr<World>& Workspace::getWorld() const {
    return world_;
}

} // namespace Core::Domain
