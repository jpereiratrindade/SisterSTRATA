#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Discursive {

/**
 * @brief Value Object representing an alleged mechanism in a discursive system.
 */
class AllegedMechanism {
public:
    AllegedMechanism() = default;
    explicit AllegedMechanism(std::string statement)
        : m_statement(std::move(statement)) {}

    const std::string& getStatement() const { return m_statement; }

    bool operator==(const AllegedMechanism& other) const {
        return m_statement == other.m_statement;
    }

    friend void to_json(nlohmann::json& j, const AllegedMechanism& obj) {
        j = nlohmann::json{{"statement", obj.m_statement}};
    }

    friend void from_json(const nlohmann::json& j, AllegedMechanism& obj) {
        obj.m_statement = j.at("statement").get<std::string>();
    }

private:
    std::string m_statement;
};

} // namespace SisterSTRATA::Observational::Discursive
