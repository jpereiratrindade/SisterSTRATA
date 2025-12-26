#pragma once

#include <vector>
#include <memory>
#include <optional>
#include "core/domain/Dataset.hpp"
#include "core/domain/World.hpp"

namespace Core::Domain {

class Workspace {
public:
    void addDataset(const std::string& name, const std::string& path_str);
    void createWorld(const std::string& name, uint32_t width, uint32_t height);

    [[nodiscard]] const std::vector<std::unique_ptr<Dataset>>& getDatasets() const;
    [[nodiscard]] const std::unique_ptr<World>& getWorld() const;

private:
    std::vector<std::unique_ptr<Dataset>> datasets_;
    std::unique_ptr<World> world_;
    
    // Simple ID generation for now
    DatasetID next_dataset_id_ = 1;
    WorldID next_world_id_ = 1;
};

} // namespace Core::Domain
