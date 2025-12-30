#pragma once

#include <string>
#include <chrono>

namespace Core::Domain::Vegetation {

/**
 * @brief Identificador explícito da hipótese ecológica declarada.
 * Permite versionamento e comparação entre cenários.
 */
class HypothesisID {
public:
    HypothesisID(std::string id) 
        : id_(std::move(id)), createdAt_(std::chrono::system_clock::now()) {}

    const std::string& getValue() const { return id_; }
    std::chrono::system_clock::time_point getTimestamp() const { return createdAt_; }

    bool operator==(const HypothesisID& other) const { return id_ == other.id_; }

private:
    std::string id_;
    std::chrono::system_clock::time_point createdAt_;
};

} // namespace Core::Domain::Vegetation
