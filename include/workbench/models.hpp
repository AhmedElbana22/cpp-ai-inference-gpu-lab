#ifndef WORKBENCH_MODELS_HPP
#define WORKBENCH_MODELS_HPP

#include <cstddef>
#include <filesystem>
#include <string>

namespace workbench
{

// Identifies a model and describes the context capacity expected by the application.
// An empty name and path mean that no model has been configured.
struct ModelConfig
{
    std::string model_name{};
    std::filesystem::path model_path{};
    std::size_t context_size{2048};
};

// Illustrative generation defaults. They are configurable examples, not universal
// recommendations for every model or task.
struct GenerationConfig
{
    std::size_t max_new_tokens{128};
    double temperature{0.7};
    double top_p{0.9};
};

// A prompt together with the generation settings requested for that prompt.
struct InferenceRequest
{
    std::string prompt{};
    GenerationConfig generation_config{};
};

// Data returned by a future inference component after generation has completed.
struct InferenceResponse
{
    std::string generated_text{};
    std::size_t prompt_tokens{0};
    std::size_t generated_tokens{0};
};

}  // namespace workbench

#endif  // WORKBENCH_MODELS_HPP
