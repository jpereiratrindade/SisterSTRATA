#pragma once

#include "application/dtos/NarrativeDTOs.hpp"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <unordered_set>
#include <cctype>
#include <algorithm>

namespace Application::Services {

/**
 * @brief Pure analytical service for building narrative context graphs
 *        and performing text tokenization / dimensional classification.
 *
 * Extracted from Session.hpp to isolate text-analysis concerns.
 * All public methods are static — no instance state needed.
 */
class NarrativeContextAnalyzer {
public:
    /**
     * @brief Tokenize text into meaningful words, filtering stopwords (PT/EN).
     */
    static std::vector<std::string> tokenizeText(const std::string& text);

    /**
     * @brief Classify the dominant knowledge dimension from a token set.
     * @return One of "ecological", "productive", "social", or "mixed".
     */
    static std::string dominantDimensionFromTokens(const std::set<std::string>& tokens);

    /**
     * @brief Build a Jaccard-similarity graph from narrative observation DTOs.
     */
    static nlohmann::json buildContextGraph(
        const std::vector<Application::DTO::NarrativeStateDTO>& narratives);
};

} // namespace Application::Services
