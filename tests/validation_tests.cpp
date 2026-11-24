#include "workbench/application.hpp"
#include "workbench/cli.hpp"
#include "workbench/inference_backend.hpp"
#include "workbench/validation.hpp"

#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>

namespace
{

std::size_t checks_run = 0;
std::size_t failures = 0;

void recordCheck(const std::string& description, bool passed)
{
    ++checks_run;

    if (passed)
    {
        std::cout << "[PASS] " << description << '\n';
    }
    else
    {
        ++failures;
        std::cout << "[FAIL] " << description << '\n';
    }
}

void expectModelValid(const std::string& description, const workbench::ModelConfig& config)
{
    recordCheck(description, !workbench::validateModelConfig(config).has_value());
}

void expectModelInvalid(const std::string& description, const workbench::ModelConfig& config)
{
    const auto error = workbench::validateModelConfig(config);
    recordCheck(description, error.has_value() && !error->empty());
}

void expectGenerationValid(
    const std::string& description,
    const workbench::GenerationConfig& config)
{
    recordCheck(description, !workbench::validateGenerationConfig(config).has_value());
}

void expectGenerationInvalid(
    const std::string& description,
    const workbench::GenerationConfig& config)
{
    const auto error = workbench::validateGenerationConfig(config);
    recordCheck(description, error.has_value() && !error->empty());
}

class AlternativeTestBackend final : public workbench::InferenceBackend
{
public:
    std::string name() const override
    {
        return "AlternativeTestBackend";
    }

    bool isInferenceReady() const noexcept override
    {
        return false;
    }

    bool isRuntimeIntegrated() const noexcept override
    {
        return true;
    }
};

void testModelConfigValidation()
{
    workbench::ModelConfig valid_config{
        "ExampleModel",
        "models/example.gguf",
        4096};
    expectModelValid("ModelConfig accepts a valid configuration", valid_config);

    workbench::ModelConfig empty_name = valid_config;
    empty_name.model_name.clear();
    expectModelInvalid("ModelConfig rejects an empty model name", empty_name);

    workbench::ModelConfig empty_path = valid_config;
    empty_path.model_path.clear();
    expectModelInvalid("ModelConfig rejects an empty model path", empty_path);

    workbench::ModelConfig zero_context = valid_config;
    zero_context.context_size = 0;
    expectModelInvalid("ModelConfig rejects a zero context size", zero_context);
}

void testGenerationConfigValidation()
{
    workbench::GenerationConfig valid_config{128, 0.7, 0.9};
    expectGenerationValid("GenerationConfig accepts valid settings", valid_config);

    workbench::GenerationConfig zero_tokens = valid_config;
    zero_tokens.max_new_tokens = 0;
    expectGenerationInvalid("GenerationConfig rejects zero maximum new tokens", zero_tokens);

    workbench::GenerationConfig negative_temperature = valid_config;
    negative_temperature.temperature = -0.1;
    expectGenerationInvalid("GenerationConfig rejects negative temperature", negative_temperature);

    workbench::GenerationConfig nan_temperature = valid_config;
    nan_temperature.temperature = std::numeric_limits<double>::quiet_NaN();
    expectGenerationInvalid("GenerationConfig rejects NaN temperature", nan_temperature);

    workbench::GenerationConfig infinite_temperature = valid_config;
    infinite_temperature.temperature = std::numeric_limits<double>::infinity();
    expectGenerationInvalid("GenerationConfig rejects infinite temperature", infinite_temperature);

    workbench::GenerationConfig zero_top_p = valid_config;
    zero_top_p.top_p = 0.0;
    expectGenerationInvalid("GenerationConfig rejects top-p equal to zero", zero_top_p);

    workbench::GenerationConfig top_p_above_one = valid_config;
    top_p_above_one.top_p = 1.01;
    expectGenerationInvalid("GenerationConfig rejects top-p greater than one", top_p_above_one);

    workbench::GenerationConfig nan_top_p = valid_config;
    nan_top_p.top_p = std::numeric_limits<double>::quiet_NaN();
    expectGenerationInvalid("GenerationConfig rejects NaN top-p", nan_top_p);

    workbench::GenerationConfig infinite_top_p = valid_config;
    infinite_top_p.top_p = std::numeric_limits<double>::infinity();
    expectGenerationInvalid("GenerationConfig rejects infinite top-p", infinite_top_p);

    workbench::GenerationConfig boundary_values{1, 0.0, 1.0};
    expectGenerationValid(
        "GenerationConfig accepts boundary values: one token, zero temperature, top-p one",
        boundary_values);
}

void testBackendDependencyInjection()
{
    static_assert(
        std::is_constructible<
            workbench::Application,
            workbench::Cli&,
            AlternativeTestBackend&,
            const workbench::ModelConfig&,
            const workbench::GenerationConfig&>::value,
        "Application must accept a different InferenceBackend implementation.");

    workbench::Cli cli;
    AlternativeTestBackend alternative_backend;
    const workbench::ModelConfig model_config{};
    const workbench::GenerationConfig generation_config{};

    // Construction verifies that Application can receive another implementation
    // through the interface without including or depending on the mock backend.
    workbench::Application application(
        cli,
        alternative_backend,
        model_config,
        generation_config);
    (void)application;

    workbench::InferenceBackend& backend_interface = alternative_backend;
    recordCheck(
        "An alternative backend satisfies the injected interface",
        backend_interface.name() == "AlternativeTestBackend" &&
            !backend_interface.isInferenceReady() &&
            backend_interface.isRuntimeIntegrated());
}

}  // namespace

int main()
{
    std::cout << "Running C++ Local LLM Inference Workbench tests...\n\n";

    testModelConfigValidation();
    testGenerationConfigValidation();
    testBackendDependencyInjection();

    std::cout << "\nTest summary: " << (checks_run - failures) << '/' << checks_run
              << " checks passed.\n";

    if (failures != 0)
    {
        std::cout << failures << " check(s) failed.\n";
        return 1;
    }

    std::cout << "All checks passed.\n";
    return 0;
}
