#include "workbench/application.hpp"
#include "workbench/mock_inference_backend.hpp"

#include <cstdlib>

int main()
{
    // The composition root creates concrete components and injects them into
    // the application. Application itself only knows the InferenceBackend API.
    workbench::Cli cli;
    workbench::MockInferenceBackend backend;

    const workbench::ModelConfig model_config{};
    const workbench::GenerationConfig generation_config{};

    workbench::Application application(
        cli,
        backend,
        model_config,
        generation_config);

    application.run();
    return EXIT_SUCCESS;
}
