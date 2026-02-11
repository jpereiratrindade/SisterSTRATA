#include "application/services/NarrativeContextAnalyzer.hpp"

namespace Application::Services {

using json = nlohmann::json;

std::vector<std::string> NarrativeContextAnalyzer::tokenizeText(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current;
    current.reserve(32);
    auto flush = [&]() {
        if (current.size() < 4) {
            current.clear();
            return;
        }
        static const std::unordered_set<std::string> stopwords = {
            "this","that","with","from","into","between","sobre","entre","para","como","when","where","which",
            "because","without","within","through","pela","pelos","pelas","pelo","dos","das","and","the","for",
            "uma","umas","uns","com","sem","depois","antes","were","was","sao","são","isso","essa","este","esta",
            "their","there","have","has","had","very","more","less","than","sobre","sobre","ainda","also","only",
            "de","do","da","em","no","na","nos","nas","por","que","se","ao","aos","as","os","um","uma","e"
        };
        if (!stopwords.contains(current)) {
            tokens.push_back(current);
        }
        current.clear();
    };

    for (char ch : text) {
        const unsigned char c = static_cast<unsigned char>(ch);
        if (std::isalnum(c) != 0) {
            current.push_back(static_cast<char>(std::tolower(c)));
        } else {
            flush();
        }
    }
    flush();
    return tokens;
}

std::string NarrativeContextAnalyzer::dominantDimensionFromTokens(const std::set<std::string>& tokens) {
    static const std::unordered_set<std::string> ecological = {
        "solo","soil","vegetacao","vegetation","ecologico","ecological","campo","pasture","forragem","forage",
        "raizes","roots","umidade","water","hidrico","hydro","clima","climate","biologica","biological",
        "micorrizica","mycorrhizal","temperatura","emissao","methane","metano","carbon","carbono"
    };
    static const std::unordered_set<std::string> productive = {
        "manejo","management","producao","production","produtividade","productivity","pastejo","grazing",
        "sistema","system","tecnico","technical","fertilizacao","fertilization","otimizacao","optimization",
        "indicador","indicator","monitoramento","monitoring"
    };
    static const std::unordered_set<std::string> social = {
        "regional","region","territorio","territory","politica","policy","institucional","institutional",
        "governanca","governance","colaboracao","collaboration","social","sociais","communities","comunidade"
    };

    size_t eco = 0, prod = 0, soc = 0;
    for (const auto& token : tokens) {
        if (ecological.contains(token)) ++eco;
        if (productive.contains(token)) ++prod;
        if (social.contains(token)) ++soc;
    }
    if (eco >= prod && eco >= soc && eco > 0) return "ecological";
    if (prod >= eco && prod >= soc && prod > 0) return "productive";
    if (soc >= eco && soc >= prod && soc > 0) return "social";
    return "mixed";
}

nlohmann::json NarrativeContextAnalyzer::buildContextGraph(
    const std::vector<Application::DTO::NarrativeStateDTO>& narratives) {

    struct ContextNodeData {
        std::string sourceId;
        size_t count = 0;
        std::set<std::string> tokens;
        std::map<std::string, size_t> intents;
        std::set<std::string> artifactIds;
        std::vector<std::string> observationIds;
    };

    std::map<std::string, ContextNodeData> grouped;
    for (const auto& dto : narratives) {
        std::string sourceId = dto.source.sourceId.empty() ? "unknown_source" : dto.source.sourceId;
        auto& item = grouped[sourceId];
        item.sourceId = sourceId;
        item.count += 1;
        item.intents[dto.intent.intentType] += 1;
        item.observationIds.push_back(dto.id);
        auto itArtifact = dto.metadata.find("iw.artifactId");
        if (itArtifact != dto.metadata.end() && !itArtifact->second.empty()) {
            item.artifactIds.insert(itArtifact->second);
        }

        std::vector<std::string> textBlocks;
        textBlocks.push_back(dto.temporalContext.label);
        for (const auto& axis : dto.axes) {
            textBlocks.push_back(axis.label);
            textBlocks.push_back(axis.description);
        }
        auto itObs = dto.metadata.find("iw.observation");
        if (itObs != dto.metadata.end()) textBlocks.push_back(itObs->second);
        auto itCtx = dto.metadata.find("iw.context");
        if (itCtx != dto.metadata.end()) textBlocks.push_back(itCtx->second);
        auto itEvidence = dto.metadata.find("iw.evidenceSnippet");
        if (itEvidence != dto.metadata.end()) textBlocks.push_back(itEvidence->second);

        for (const auto& block : textBlocks) {
            for (const auto& token : tokenizeText(block)) {
                item.tokens.insert(token);
            }
        }
    }

    json graph;
    graph["distanceType"] = "epistemic_narrative_jaccard_v1";
    graph["causalInterpretationAllowed"] = false;
    graph["nodes"] = json::array();
    graph["edges"] = json::array();

    std::vector<ContextNodeData> nodes;
    nodes.reserve(grouped.size());
    for (const auto& [_, node] : grouped) {
        nodes.push_back(node);
    }

    auto topTokens = [](const std::set<std::string>& tokenSet, size_t maxCount) {
        std::vector<std::string> out;
        out.reserve(std::min(maxCount, tokenSet.size()));
        for (const auto& token : tokenSet) {
            out.push_back(token);
            if (out.size() >= maxCount) break;
        }
        return out;
    };

    auto toVector = [](const std::set<std::string>& values) {
        std::vector<std::string> out;
        out.reserve(values.size());
        for (const auto& value : values) {
            out.push_back(value);
        }
        return out;
    };

    for (const auto& node : nodes) {
        std::string dominantIntent = "unknown";
        size_t best = 0;
        for (const auto& [intent, count] : node.intents) {
            if (count > best) {
                best = count;
                dominantIntent = intent;
            }
        }

        graph["nodes"].push_back({
            {"id", node.sourceId},
            {"label", node.sourceId},
            {"narrativeCount", node.count},
            {"dominantIntent", dominantIntent},
            {"dominantDimension", dominantDimensionFromTokens(node.tokens)},
            {"topTokens", topTokens(node.tokens, 10)},
            {"artifactIds", toVector(node.artifactIds)},
            {"observationIds", node.observationIds}
        });
    }

    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = i + 1; j < nodes.size(); ++j) {
            const auto& a = nodes[i].tokens;
            const auto& b = nodes[j].tokens;
            if (a.empty() || b.empty()) continue;

            size_t intersection = 0;
            for (const auto& token : a) {
                if (b.contains(token)) ++intersection;
            }
            const size_t uni = a.size() + b.size() - intersection;
            if (uni == 0) continue;

            const double similarity = static_cast<double>(intersection) / static_cast<double>(uni);
            const double distance = 1.0 - similarity;

            if (similarity <= 0.0) continue;
            graph["edges"].push_back({
                {"source", nodes[i].sourceId},
                {"target", nodes[j].sourceId},
                {"similarity", similarity},
                {"distance", distance},
                {"sharedTokens", intersection}
            });
        }
    }

    return graph;
}

} // namespace Application::Services
