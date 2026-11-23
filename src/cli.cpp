#include "workbench/cli.hpp"

#include <iostream>

namespace workbench
{

void Cli::printWelcomeMessage() const
{
    std::cout << "Welcome to C++ Local LLM Inference Workbench!\n\n";
    std::cout << "Available commands:\n";
    std::cout << "- help\n";
    std::cout << "- status\n";
    std::cout << "- exit\n";
}

void Cli::printHelp() const
{
    std::cout << "\nAvailable commands:\n";
    std::cout << "- help    Show available commands\n";
    std::cout << "- status  Display application information\n";
    std::cout << "- exit    Exit the application\n";
}

bool Cli::readCommand(std::string& command) const
{
    std::cout << "\n> " << std::flush;
    return static_cast<bool>(std::getline(std::cin, command));
}

void Cli::printStatus(
    const ModelConfig& model_config,
    const GenerationConfig& generation_config,
    const std::string& backend_name,
    bool inference_ready,
    bool runtime_integrated) const
{
    const bool model_is_configured =
        !model_config.model_name.empty() && !model_config.model_path.empty();

    std::cout << "\nApplication: C++ Local LLM Inference Workbench\n";
    std::cout << "Project: C++ AI Inference & GPU Acceleration Lab\n";
    std::cout << "Development stage: Session 04 - Application Architecture & Abstraction\n";
    std::cout << "Status: Initialized\n";
    std::cout << "Model configured: " << (model_is_configured ? "Yes" : "No") << '\n';

    if (model_is_configured)
    {
        std::cout << "Model name: " << model_config.model_name << '\n';
        std::cout << "Model path: " << model_config.model_path.string() << '\n';
    }
    else
    {
        std::cout << "Model name: Not configured\n";
        std::cout << "Model path: Not configured\n";
    }

    std::cout << "Default context size: " << model_config.context_size << " tokens\n";
    std::cout << "Illustrative generation defaults:\n";
    std::cout << "- max_new_tokens: " << generation_config.max_new_tokens << '\n';
    std::cout << "- temperature: " << generation_config.temperature << '\n';
    std::cout << "- top_p: " << generation_config.top_p << '\n';
    std::cout << "Backend: " << backend_name << '\n';
    std::cout << "Inference readiness: " << (inference_ready ? "Ready" : "Not ready") << '\n';
    std::cout << "Real inference runtime: "
              << (runtime_integrated ? "Integrated" : "Not integrated") << '\n';
}

void Cli::printUnknownCommandMessage() const
{
    std::cout << "\nError: Unknown command.\n";
    std::cout << "Type 'help' to see available commands.\n";
}

void Cli::printShutdownMessage() const
{
    std::cout << "\nShutting down application...\n";
}

void Cli::printEndOfInputMessage() const
{
    std::cout << "\nEnd of input detected. Shutting down application...\n";
}

}  // namespace workbench
