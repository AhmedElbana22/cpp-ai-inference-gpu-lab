#ifndef WORKBENCH_APPLICATION_HPP
#define WORKBENCH_APPLICATION_HPP

#include "cli.hpp"
#include "inference_backend.hpp"
#include "models.hpp"

#include <string>

namespace workbench
{

// Coordinates the application workflow while depending on the backend
// abstraction rather than any concrete backend implementation.
class Application
{
public:
    Application(
        Cli& cli,
        InferenceBackend& backend,
        const ModelConfig& model_config,
        const GenerationConfig& generation_config);

    void run();

private:
    bool handleCommand(const std::string& input);

    Cli& cli_;
    InferenceBackend& backend_;
    const ModelConfig& model_config_;
    const GenerationConfig& generation_config_;
};

}  // namespace workbench

#endif  // WORKBENCH_APPLICATION_HPP
