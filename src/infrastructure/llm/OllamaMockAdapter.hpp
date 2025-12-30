#pragma once

#include "application/ports/ILLMService.hpp"
#include <thread>
#include <atomic>

namespace Infrastructure::LLM {

/**
 * @brief Mock implementation of ILLMService for UI development and testing.
 * Simulates network latency and returns predefined hermeneutic responses.
 */
class OllamaMockAdapter : public Application::Ports::ILLMService {
public:
    OllamaMockAdapter() = default;
    ~OllamaMockAdapter() override;

    void requestCompletion(const std::vector<Application::Ports::LLMMessage>& messages, 
                           Application::Ports::ILLMService::CompletionCallback callback) override;

    bool isAvailable() const override { return true; }

private:
    std::thread workerThread_;
    std::atomic<bool> isBusy_{false};
};

} // namespace Infrastructure::LLM
