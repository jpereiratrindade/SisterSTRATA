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
    // Attempt to load from the DDD document if available
    std::ifstream file("DDD_Cognitive_Assistance_Qwen_STRATA_v1.0.txt");
    if (file.is_open()) {
        std::stringstream ss;
        ss << file.rdbuf();
        systemPrompt_ = ss.str();
    } else {
        systemPrompt_ = "Você é um assistente cognitivo para o sistema SisterSTRATA.";
    }
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
        cli.set_read_timeout(60); // LLMs can be slow

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

        req.callback(response);
    }
}

} // namespace Infrastructure::LLM
