#include "OllamaAdapter.hpp"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

namespace Infrastructure::LLM {

OllamaAdapter::OllamaAdapter() {
    loadSystemPrompt();
    workerThread_ = std::thread(&OllamaAdapter::processRequests, this);
}

OllamaAdapter::~OllamaAdapter() {
    running_ = false;
    cv_.notify_one();
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

void OllamaAdapter::loadSystemPrompt() {
    // The "Contrato Moral" (Canonical System Prompt) from DDD_Cognitive_Bridge.md
    systemPrompt_ = 
        "You are an interpretative cognitive assistant integrated into the STRATA platform.\n\n"
        "You do NOT:\n"
        "- modify system states\n"
        "- infer bio-physical causality\n"
        "- prescribe actions\n"
        "- validate recommendations\n"
        "- make decisions\n\n"
        "You DO:\n"
        "- observe narrative states\n"
        "- compare discursive systems\n"
        "- interpret recommendation trajectories\n"
        "- highlight patterns, tensions and consistencies\n"
        "- clearly state limits and uncertainty\n\n"
        "You operate ONLY on textual representations explicitly provided.\n"
        "Everything you produce is interpretative support, not scientific output.\n\n"
        "REGRA DE SEGURANÇA INEGOCIÁVEL:\n"
        "Nunca inicie respostas com 'o ideal seria', 'recomenda-se que' ou 'o sistema deveria'.\n"
        "Use sempre: 'Nos discursos observados...', 'Uma interpretação possível é...' ou similar.\n\n"
        "IDIOMA:\n"
        "Responda SEMPRE em Português Brasileiro (pt-BR).";
}

void OllamaAdapter::requestCompletion(const std::vector<Application::Ports::LLMMessage>& messages, 
                                     Application::Ports::ILLMService::CompletionCallback callback) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        requestQueue_.push({messages, callback});
    }
    cv_.notify_one();
}

bool OllamaAdapter::isAvailable() const {
    httplib::Client cli(baseUrl_);
    if (auto res = cli.Get("/api/tags")) {
        if (res->status == 200) {
            try {
                auto tags = json::parse(res->body);
                std::vector<std::string> priorities = {
                    "qwen2.5:72b", "qwen2.5:32b", "qwen2.5:14b", "qwen2.5:7b"
                };
                
                std::string foundModel = "";
                for (const auto& p : priorities) {
                    for (const auto& m : tags["models"]) {
                        std::string name = m["name"];
                        if (name.find(p) != std::string::npos) {
                            foundModel = name;
                            break;
                        }
                    }
                    if (!foundModel.empty()) break;
                }

                if (!foundModel.empty()) {
                    // Update model name dynamically
                    modelName_ = foundModel;
                    return true;
                }
            } catch (...) {}
        }
    }
    return false;
}

void OllamaAdapter::processRequests() {
    while (running_) {
        Request req;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            cv_.wait(lock, [this] { return !requestQueue_.empty() || !running_; });
            
            if (!running_) break;
            
            req = std::move(requestQueue_.front());
            requestQueue_.pop();
        }

        // Execute HTTP Request
        httplib::Client cli(baseUrl_);
        cli.set_read_timeout(120); // LLMs can be slow (14b requires more time)

        std::cout << "[Infrastructure] Sending request to Ollama (" << modelName_ << ")..." << std::endl;

        json payload;
        payload["model"] = modelName_;
        payload["stream"] = false;
        
        json msgArray = json::array();
        
        // Add System Prompt
        msgArray.push_back({{"role", "system"}, {"content", systemPrompt_}});
        
        // Add Conversation History
        for (const auto& m : req.messages) {
            std::string roleStr;
            switch (m.role) {
                case Application::Ports::LLMRole::System: roleStr = "system"; break;
                case Application::Ports::LLMRole::User: roleStr = "user"; break;
                case Application::Ports::LLMRole::Assistant: roleStr = "assistant"; break;
            }
            msgArray.push_back({{"role", roleStr}, {"content", m.content}});
        }
        
        payload["messages"] = msgArray;

        Application::Ports::ILLMService::Response response;
        if (auto res = cli.Post("/api/chat", payload.dump(), "application/json")) {
            if (res->status == 200) {
                try {
                    auto resJson = json::parse(res->body);
                    response.success = true;
                    response.content = resJson["message"]["content"];
                } catch (const std::exception& e) {
                    response.success = false;
                    response.errorMessage = "Failed to parse JSON: " + std::string(e.what());
                }
            } else {
                response.success = false;
                response.errorMessage = "Ollama returned error: " + std::to_string(res->status);
            }
        } else {
            response.success = false;
            response.errorMessage = "Failed to connect to Ollama at " + baseUrl_;
        }

        if (response.success) {
            std::cout << "[Infrastructure] Ollama response received successfully." << std::endl;
        } else {
            std::cerr << "[Infrastructure] Ollama request failed: " << response.errorMessage << std::endl;
        }
        
        req.callback(response);
    }
}

} // namespace Infrastructure::LLM
