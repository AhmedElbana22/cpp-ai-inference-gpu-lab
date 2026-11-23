#ifndef WORKBENCH_CLI_HPP
#define WORKBENCH_CLI_HPP

#include "models.hpp"

#include <string>

namespace workbench
{

// Owns terminal input and output. It does not process application commands or
// depend on a concrete inference backend.
class Cli
{
public:
    void printWelcomeMessage() const;
    void printHelp() const;

    // Prints the prompt and reads a complete line. Returns false when EOF occurs.
    bool readCommand(std::string& command) const;

    void printStatus(
        const ModelConfig& model_config,
        const GenerationConfig& generation_config,
        const std::string& backend_name,
        bool inference_ready,
        bool runtime_integrated) const;

    void printUnknownCommandMessage() const;
    void printShutdownMessage() const;
    void printEndOfInputMessage() const;
};

}  // namespace workbench

#endif  // WORKBENCH_CLI_HPP
