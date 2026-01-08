#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Discursive {

/**
 * @brief Value Object representing a declared problem in a discursive system.
 */
class DeclaredProblem {
public:
    DeclaredProblem() = default;
    explicit DeclaredProblem(std::string statement)
        : m_statement(std::move(statement)) {}

    const std::string& getStatement() const { return m_statement; }

    bool operator==(const DeclaredProblem& other) const {
        return m_statement == other.m_statement;
    }

    friend void to_json(nlohmann::json& j, const DeclaredProblem& obj) {
        j = nlohmann::json{{"statement", obj.m_statement}};
    }

    friend void from_json(const nlohmann::json& j, DeclaredProblem& obj) {
        obj.m_statement = j.at("statement").get<std::string>();
    }

private:
    std::string m_statement;
};

} // namespace SisterSTRATA::Observational::Discursive
