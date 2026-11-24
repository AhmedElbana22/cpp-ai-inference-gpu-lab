#ifndef WORKBENCH_VALIDATION_HPP
#define WORKBENCH_VALIDATION_HPP

#include "workbench/models.hpp"

#include <optional>
#include <string>

namespace workbench
{

// Returns std::nullopt when the model configuration is valid; otherwise returns
// the first validation error. This checks configuration values only and does
// not check whether the model file exists or can be loaded.
std::optional<std::string> validateModelConfig(const ModelConfig& config);

// Returns std::nullopt when the generation configuration is valid; otherwise
// returns the first validation error.
std::optional<std::string> validateGenerationConfig(const GenerationConfig& config);

}  // namespace workbench

#endif  // WORKBENCH_VALIDATION_HPP
