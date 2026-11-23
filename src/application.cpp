#include "../include/workbench/application.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace workbench
{
namespace
{

std::string trim(const std::string& text)
{
    const auto first = std::find_if_not(
        text.begin(),
        text.end(),
        [](unsigned char character) { return std::isspace(character) != 0; });

    if (first == text.end())
    {
        return {};
    }

    const auto last = std::find_if_not(
        text.rbegin(),
        text.rend(),
        [](unsigned char character) { return std::isspace(character) != 0; });

    return std::string(first, last.base());
}

std::string normalizeCommand(const std::string& command)
{
    std::string normalized = trim(command);

    std::transform(
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });

    return normalized;
}

}  // namespace

Application::Application(
    Cli& cli,
    InferenceBackend& backend,
    const ModelConfig& model_config,
    const GenerationConfig& generation_config)
    : cli_(cli),
      backend_(backend),
      model_config_(model_config),
      generation_config_(generation_config)
{
}

void Application::run()
{
    cli_.printWelcomeMessage();

    std::string input;

    while (true)
    {
        if (!cli_.readCommand(input))
        {
            cli_.printEndOfInputMessage();
            break;
        }

        if (!handleCommand(input))
        {
            break;
        }
    }
}

bool Application::handleCommand(const std::string& input)
{
    const std::string command = normalizeCommand(input);

    if (command.empty())
    {
        return true;
    }

    if (command == "help")
    {
        cli_.printHelp();
    }
    else if (command == "status")
    {
        cli_.printStatus(
            model_config_,
            generation_config_,
            backend_.name(),
            backend_.isInferenceReady(),
            backend_.isRuntimeIntegrated());
    }
    else if (command == "exit")
    {
        cli_.printShutdownMessage();
        return false;
    }
    else
    {
        cli_.printUnknownCommandMessage();
    }

    return true;
}

}  // namespace workbench
