#include "OllamaMockAdapter.hpp"
#include <chrono>

namespace Infrastructure::LLM {

OllamaMockAdapter::~OllamaMockAdapter() {
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

void OllamaMockAdapter::requestCompletion(
    const std::vector<Application::Ports::LLMMessage>& messages, 
    Application::Ports::ILLMService::CompletionCallback callback) 
{
    // Simple mock logic: wait 2 seconds and return a response based on keywords or generic.
    if (workerThread_.joinable()) {
        workerThread_.join();
    }

    workerThread_ = std::thread([this, callback]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        Application::Ports::ILLMService::Response response;
        response.success = true;
        response.content = "[FATOS DO SISTEMA]\nIdentificada transição de cobertura campestre para florestal natural.\nIntensidade de coerência calculada em 0.620.\n\n[INTERPRETAÇÃO COGNITIVA]\nEsta transição sugere um processo de adensamento sucessional estruturado. A manutenção de uma coerência média (0.620) pode indicar que, apesar da mudança de fisionomia, a configuração espacial e os fluxos ecológicos subjacentes permanecem estáveis.\n\n[HIPÓTESE DE RESILIÊNCIA]\nO padrão é compatível com uma trajetória de recuperação de biomassa. A estabilidade estrutural observada sugere uma baixa volatilidade no processo de transição, o que pode indicar uma resiliência robusta do ecótono frente às mudanças observadas.";
        
        callback(response);
    });
}

} // namespace Infrastructure::LLM
