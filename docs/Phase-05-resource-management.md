# Phase 05 - Resource Management, Error Handling & Testing

## Scope

This Phase extends the Phase 04 application without replacing its architecture, C++17 baseline, or executable target. It makes object lifetime and ownership explicit, adds reusable validation for the existing configuration structures, and introduces a small test executable registered with CTest. The mock backend remains a placeholder: no model is loaded and no inference is performed.

## Resource management in C++

A resource is something a program acquires and must eventually release or stop using. Examples include dynamically allocated memory, file handles, sockets, locks, and other operating-system resources. Resource management defines who is responsible for that lifetime and how cleanup occurs.

### Stack and heap allocation

The application currently creates its long-lived objects as local variables in `main()`:

```cpp
workbench::Cli cli;
workbench::MockInferenceBackend backend;
const workbench::ModelConfig model_config{};
const workbench::GenerationConfig generation_config{};

workbench::Application application(
    cli, backend, model_config, generation_config);
application.run();
```

These objects have automatic storage duration (commonly described as stack lifetime). They are constructed when execution reaches their declarations and destroyed automatically when their scope ends. This is appropriate because the objects are needed only while `main()` runs.

Dynamic allocation uses the free store (commonly called the heap), for example through `std::make_unique`. It is useful when an object's lifetime must be dynamic, ownership must be transferred, or a polymorphic object must be created behind a pointer. It is not inherently faster or more professional. The current application does not need dynamic object lifetime, so stack objects avoid unnecessary allocation and ownership complexity.

### Object lifetime and references

`Application` stores non-owning references to `Cli`, `InferenceBackend`, `ModelConfig`, and `GenerationConfig`. A reference does not extend the lifetime of the object it refers to. The composition root must therefore ensure that every dependency outlives the `Application` that uses it.

The declarations in `main()` satisfy that requirement. The application is declared after its dependencies, so it is destroyed first when `main()` exits. The referenced CLI, backend, and configuration objects are still alive during the application's entire lifetime. Returning from `main()` then destroys those dependencies in reverse declaration order.

The distinction is:

- **Owner:** responsible for creating an object and ensuring it is destroyed at the right time.
- **Non-owning reference:** provides access to an object but does not control its lifetime.

The current composition root owns the objects through ordinary automatic scope. `Application` only observes them through references. There are no owning raw pointers in the application.

## RAII

RAII means *Resource Acquisition Is Initialization*. A resource is acquired and tied to an object's lifetime. When the object is destroyed, its destructor releases the resource. This gives cleanup a predictable scope-based rule, including when a function returns early or an exception leaves the scope.

The existing models already use standard-library value types with automatic lifetime management:

```cpp
struct ModelConfig
{
    std::string model_name{};
    std::filesystem::path model_path{};
    std::size_t context_size{2048};
};
```

`std::string` owns its character storage and `std::filesystem::path` manages its path representation. Their destructors handle their internal resources; application code does not manually allocate or free their memory.

When model-file loading is introduced, `std::ifstream` or another suitable RAII wrapper can own an open file handle and close it when it leaves scope. This Phase does not add an artificial file operation because no model-loading behavior exists yet. The same principle applies to `std::ofstream` when an actual output-file requirement appears.

RAII is important in production C++ because it makes cleanup local, predictable, and resistant to early returns and exceptions. It reduces leaks and makes ownership easier to review.

## Raw pointers and smart pointers

A raw pointer is an address. By itself, it does not say whether the pointed-to object is owned, shared, optional, or already destroyed. Raw pointers are useful for some non-owning or low-level APIs, but an owning raw pointer requires manually coordinated deletion and is easy to misuse.

The standard smart pointers express ownership more clearly:

- **`std::unique_ptr<T>`:** exclusive ownership. One smart pointer owns the object; it destroys it automatically. Ownership can be moved but not copied. It is a good default when dynamic ownership is required.
- **`std::shared_ptr<T>`:** shared ownership. The object remains alive while at least one owning `shared_ptr` remains. A control block tracks the shared ownership count. It is appropriate only when multiple components genuinely need to co-own an object's lifetime.
- **`std::weak_ptr<T>`:** non-owning observation of an object managed by `shared_ptr`. It does not keep the object alive and must be checked/locked before use. It is useful for observing shared objects without creating ownership cycles.

These types solve different ownership problems; they are not interchangeable convenience wrappers. This application uses none of them yet. Stack-owned dependencies and non-owning references already model the current lifetime relationship directly. If a future backend must be selected and created dynamically behind `InferenceBackend`, `std::unique_ptr<InferenceBackend>` may be appropriate. Shared ownership should be considered only if there is a demonstrated need for multiple owners.

## Configuration validation

Validation checks whether input values satisfy the rules currently defined by the application. It is separated from the data-only structures and from CLI input/output.

The reusable declarations are in `include/workbench/validation.hpp`; their definitions are in `src/validation.cpp`. Both functions accept `const` references because they inspect the configuration without modifying or copying it:

```cpp
std::optional<std::string> validateModelConfig(
    const ModelConfig& config);

std::optional<std::string> validateGenerationConfig(
    const GenerationConfig& config);
```

### Model configuration rules

`validateModelConfig()` checks, in order:

1. `model_name` is not empty.
2. `model_path` is not empty.
3. `context_size` is greater than zero.

It does **not** check whether the path exists, whether it is readable, or whether the file is a compatible model. Those are model-loading/runtime checks and are deliberately deferred. An empty default `ModelConfig{}` represents the application's current *unconfigured* state; consequently, it is not a valid configured model and the validation function reports the first missing field.

### Generation configuration rules

`validateGenerationConfig()` checks:

1. `max_new_tokens` is greater than zero.
2. `temperature` is finite and non-negative.
3. `top_p` is finite, greater than zero, and less than or equal to one.

`std::isfinite()` rejects NaN and positive or negative infinity. A comparison alone is not sufficient for NaN because comparisons with NaN are false in ways that can bypass ordinary range checks.

The generation defaults in `models.hpp` remain illustrative examples. Validation enforces the explicit application rules above; it does not claim that those settings are optimal for every model or task.

## Validation result: `std::optional<std::string>`

The functions use `std::optional<std::string>`, available in C++17:

- `std::nullopt` means that validation succeeded.
- A string means validation failed; the string explains the first failure found.

This is a small return-value contract and avoids exceptions for expected invalid input. An alternative would be a `bool` plus an output parameter for the error message, or a custom result type containing multiple errors. Those approaches may be useful if the application later needs to report several errors at once, attach error codes, or carry richer diagnostic information. They are not necessary for the current two validators.

The validators are reusable and independently tested. The present CLI has no command that edits model or generation settings, so the validators are not called by `status`. In particular, the status output must continue to distinguish an unconfigured model from a loaded model. A future configuration-entry or model-loading workflow can call these functions at its input boundary.

## Error handling principles

It is useful to distinguish three broad categories:

1. **Expected validation failures:** user-provided or configured values do not meet defined rules. Return a validation result such as `std::optional<std::string>`; do not throw an exception for an ordinary invalid value.
2. **Recoverable application errors:** an operation cannot proceed, but the application can report the problem and continue or let the user retry. A future runtime API may use a result type or a documented exception boundary, depending on the runtime integration.
3. **Unexpected exceptional failures:** conditions such as unexpected allocation failure or failures from operations that report errors through exceptions. Exceptions can communicate these failures up the call stack. They should be caught at a boundary that can meaningfully recover, report, or terminate; they should not be silently swallowed.

This Phase does not add generic `try`/`catch` blocks because the current CLI and validators do not require them. Model-file access, backend initialization, and inference-specific failure behavior will be designed when those operations are introduced. No exception hierarchy is added prematurely.

## Unit testing and CTest

The project now has a `tests/validation_tests.cpp` test executable. It uses a small local test runner instead of an external framework. Each check prints a descriptive `[PASS]` or `[FAIL]` line. The process exits with code zero only when all checks pass; otherwise it returns a non-zero exit code.

The tests cover:

### `ModelConfig`

- A valid configuration.
- Empty model name.
- Empty model path.
- Zero context size.

### `GenerationConfig`

- A valid configuration.
- Zero maximum new tokens.
- Negative temperature.
- NaN temperature.
- Infinite temperature.
- Top-p equal to zero.
- Top-p greater than one.
- NaN and infinite top-p.
- Boundary values: one maximum token, zero temperature, and top-p equal to one.

The tests do not use `assert()`. Standard assertions can be compiled out when `NDEBUG` is defined in Release builds, which could make a test executable appear to pass without evaluating its checks. The explicit runner evaluates each condition in every build configuration.

### Backend dependency check

The test file defines a small `AlternativeTestBackend` that implements `InferenceBackend` without inheriting from or including `MockInferenceBackend`. A compile-time constructibility check and construction of `Application` with this alternate implementation verify that the application's constructor accepts the abstraction. The test also calls the alternate backend through an `InferenceBackend&` to verify its virtual interface. It does not run the interactive application loop or simulate a terminal Phase.

### CTest integration

`include(CTest)` enables CTest and provides the `BUILD_TESTING` option. When testing is enabled, CMake builds `workbench_validation_tests` and registers it with `add_test()`. The existing application target is retained. Both targets use C++17, disable compiler extensions, and receive the established warning options.

CTest provides a standard way for IDEs and build pipelines to discover and execute tests. An external framework such as GoogleTest could offer fixtures, richer assertions, parameterized tests, and reporting. The current number and complexity of checks do not justify a new package dependency, so the local runner is sufficient for this milestone.

## Alternatives and trade-offs

- **Heap allocation for all dependencies:** rejected because current object lifetimes are bounded by `main()` and need no dynamic ownership.
- **`unique_ptr` for the backend immediately:** possible, especially for future runtime selection, but not needed while `main()` selects one known stack object.
- **`shared_ptr` ownership:** rejected because no components need to co-own the backend or other dependencies.
- **Exceptions for invalid configuration:** rejected because invalid settings are expected validation outcomes, not exceptional control flow.
- **A custom validation-result class:** could carry multiple errors or error codes, but `optional<string>` expresses the current first-error contract clearly.
- **A third-party test framework:** offers more features, but would add an external dependency for a small number of straightforward checks.
- **Manual CLI tests as unit tests:** avoided because terminal interaction is not the target of configuration-validation tests. The application command behavior remains separately exercised through normal CLI execution.

## Build and test

Configure and build from the repository root:

```sh
cmake -S . -B build
cmake --build build --config Release
```

Run the registered tests:

```sh
cmake -E chdir build ctest -C Release --output-on-failure
```

To configure without building the test target:

```sh
cmake -S . -B build -DBUILD_TESTING=OFF
```

The application keeps the original target name, `cpp_local_llm_inference_workbench`, and the project remains on C++17. No external testing package, model runtime, Python, PyTorch, Transformers, CUDA, or llama.cpp is added.

## Extension path

Later Phases can introduce model-file access, backend initialization, inference requests, and runtime-specific error handling. Those operations should use clear ownership and RAII for any resources they acquire. The current validators and tests provide a small foundation without anticipating the details of a future runtime API.
