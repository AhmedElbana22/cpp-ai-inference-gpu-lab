#ifndef WORKBENCH_MOCK_INFERENCE_BACKEND_HPP
#define WORKBENCH_MOCK_INFERENCE_BACKEND_HPP

#include "workbench/inference_backend.hpp"

namespace workbench
{

// Placeholder backend used to exercise the architecture before a real runtime
// is integrated. It deliberately performs no model loading or inference.
class MockInferenceBackend final : public InferenceBackend
{
public:
    std::string name() const override;
    bool isInferenceReady() const noexcept override;
    bool isRuntimeIntegrated() const noexcept override;
};

}  // namespace workbench

#endif  // WORKBENCH_MOCK_INFERENCE_BACKEND_HPP
