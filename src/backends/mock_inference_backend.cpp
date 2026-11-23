#include "workbench/mock_inference_backend.hpp"

namespace workbench
{

std::string MockInferenceBackend::name() const
{
    return "MockInferenceBackend";
}

bool MockInferenceBackend::isInferenceReady() const noexcept
{
    return false;
}

bool MockInferenceBackend::isRuntimeIntegrated() const noexcept
{
    return false;
}

}  // namespace workbench
