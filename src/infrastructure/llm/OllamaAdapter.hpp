#pragma once

#include "application/ports/ILLMService.hpp"
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>

namespace Infrastructure::LLM {

/**
 * @brief Real implementation of ILLMService using Ollama's REST API.
 */
class OllamaAdapter : public Application::Ports::ILLMService {
public:
    OllamaAdapter();
    ~OllamaAdapter() override;

    void requestCompletion(const std::vector<Application::Ports::LLMMessage>& messages, 
                           Application::Ports::ILLMService::CompletionCallback callback) override;

    bool isAvailable() const override;
    std::string getModelName() const override { return modelName_; }

private:
    struct Request {
        std::vector<Application::Ports::LLMMessage> messages;
        Application::Ports::ILLMService::CompletionCallback callback;
    };

    std::string systemPrompt_;
    void loadSystemPrompt();
    
    // Worker Thread Logic
    std::thread workerThread_;
    std::atomic<bool> running_{true};
    std::queue<Request> requestQueue_;
    std::mutex queueMutex_;
    std::condition_variable cv_;

    void processRequests();
    
    std::string baseUrl_ = "http://127.0.0.1:11434";
    mutable std::string modelName_ = "qwen2.5:7b";
};

} // namespace Infrastructure::LLM
