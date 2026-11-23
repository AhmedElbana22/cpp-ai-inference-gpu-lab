#ifndef WORKBENCH_INFERENCE_BACKEND_HPP
#define WORKBENCH_INFERENCE_BACKEND_HPP

#include <string>

namespace workbench
{

// Describes the information the application needs from an inference backend.
// Concrete backends provide these details without exposing runtime-specific APIs.
class InferenceBackend
{
public:
    virtual ~InferenceBackend() = default;

    virtual std::string name() const = 0;
    virtual bool isInferenceReady() const noexcept = 0;
    virtual bool isRuntimeIntegrated() const noexcept = 0;
};

}  // namespace workbench

#endif  // WORKBENCH_INFERENCE_BACKEND_HPP


// this is not allowed => InferenceBackend backend; // Error: cannot instantiate abstract class
