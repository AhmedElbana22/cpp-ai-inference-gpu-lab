#include "workbench/validation.hpp"

#include <cmath>

namespace workbench
{

std::optional<std::string> validateModelConfig(const ModelConfig& config)
{
    if (config.model_name.empty())
    {
        return "Model name must not be empty.";
    }

    if (config.model_path.empty())
    {
        return "Model path must not be empty.";
    }

    if (config.context_size == 0)
    {
        return "Context size must be greater than zero.";
    }

    return std::nullopt;
}

std::optional<std::string> validateGenerationConfig(const GenerationConfig& config)
{
    if (config.max_new_tokens == 0)
    {
        return "Maximum new tokens must be greater than zero.";
    }

    if (!std::isfinite(config.temperature) || config.temperature < 0.0)
    {
        return "Temperature must be finite and non-negative.";
    }

    if (!std::isfinite(config.top_p) || config.top_p <= 0.0 || config.top_p > 1.0)
    {
        return "Top-p must be finite, greater than zero, and less than or equal to one.";
    }

    return std::nullopt;
}

}  // namespace workbench
