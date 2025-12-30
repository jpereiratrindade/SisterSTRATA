#pragma once

#include <string>
#include <functional>
#include <vector>

namespace Application::Ports {

/**
 * @brief Identifies the role of a message in the conversation.
 */
enum class LLMRole {
    System,
    User,
    Assistant
};

/**
 * @brief Represents a single message in the LLM context.
 */
struct LLMMessage {
    LLMRole role;
    std::string content;
};

/**
 * @brief Interface for Large Language Model services (e.g., Qwen via Ollama).
 * This follows the Ports & Adapters architecture to isolate the application from specific LLM providers.
 */
class ILLMService {
public:
    virtual ~ILLMService() = default;

    /**
     * @brief Result of an LLM request.
     */
    struct Response {
        bool success;
        std::string content;
        std::string errorMessage;
    };

    using CompletionCallback = std::function<void(const Response&)>;

    /**
     * @brief Asynchronously requests a completion from the LLM.
     * @param messages The conversation history/context.
     * @param callback Function called when the response is ready.
     */
    virtual void requestCompletion(const std::vector<LLMMessage>& messages, CompletionCallback callback) = 0;

    /**
     * @brief Synchronously checks if the service is available (e.g., Ollama is running).
     */
    virtual bool isAvailable() const = 0;
};

} // namespace Application::Ports
