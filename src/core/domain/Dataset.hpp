#pragma once

#include <cstdint>
#include <string>
#include "core/value_objects/FilePath.hpp"

namespace Core::Domain {

using DatasetID = uint32_t;

class Dataset {
public:
    Dataset(DatasetID id, const std::string& name, const ValueObjects::FilePath& path)
        : id_(id), name_(name), path_(path) {}

    [[nodiscard]] DatasetID getId() const { return id_; }
    [[nodiscard]] const std::string& getName() const { return name_; }
    [[nodiscard]] const ValueObjects::FilePath& getPath() const { return path_; }

private:
    DatasetID id_;
    std::string name_;
    ValueObjects::FilePath path_;
};

} // namespace Core::Domain
