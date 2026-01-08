#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Discursive {

/**
 * @brief Value Object representing an expected effect in a discursive system.
 */
class ExpectedEffect {
public:
    ExpectedEffect() = default;
    explicit ExpectedEffect(std::string statement)
        : m_statement(std::move(statement)) {}

    const std::string& getStatement() const { return m_statement; }

    bool operator==(const ExpectedEffect& other) const {
        return m_statement == other.m_statement;
    }

    friend void to_json(nlohmann::json& j, const ExpectedEffect& obj) {
        j = nlohmann::json{{"statement", obj.m_statement}};
    }

    friend void from_json(const nlohmann::json& j, ExpectedEffect& obj) {
        obj.m_statement = j.at("statement").get<std::string>();
    }

private:
    std::string m_statement;
};

} // namespace SisterSTRATA::Observational::Discursive
