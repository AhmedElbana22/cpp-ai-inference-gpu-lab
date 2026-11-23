# Phase 03 — Application Data Modeling

## Scope

This Phase extends the existing **C++ Local LLM Inference Workbench** with four reusable data structures. They describe model configuration, generation settings, an inference request, and an inference response. The existing interactive CLI remains in `src/main.cpp`; no model is loaded and no inference is performed. The project continues to use C++17 and the existing executable target.

## Why the application needs data models

As the application grows, several components will need to exchange the same information. Dedicated data structures give that information a clear shape and consistent field names. They reduce duplicated fields and make the intended inputs and outputs visible in function signatures. These structures are data contracts only: they do not perform model loading, tokenization, generation, CUDA work, or CLI input handling.

## The four structures

All four structures are declared in `include/workbench/models.hpp` inside the `workbench` namespace.

### `ModelConfig`

`ModelConfig` describes which model the application is configured to use and the context capacity selected for it:

```cpp
struct ModelConfig
{
    std::string model_name{};
    std::filesystem::path model_path{};
    std::size_t context_size{2048};
};
```

An empty `model_name` and `model_path` indicate that no model is configured. `context_size` has an illustrative default of 2048 tokens; an actual model and runtime may impose different limits. `std::filesystem::path` represents a filesystem location more appropriately than a plain string and supports platform-specific path conventions.

### `GenerationConfig`

`GenerationConfig` groups the settings that control a future text-generation operation:

```cpp
struct GenerationConfig
{
    std::size_t max_new_tokens{128};
    double temperature{0.7};
    double top_p{0.9};
};
```

The defaults are configurable examples for demonstrating the model, not universally optimal settings. The correct values depend on the model, task, runtime, and application requirements. This Phase stores the values but does not enforce ranges or apply them to a generation engine.

### `InferenceRequest`

`InferenceRequest` combines the user's prompt with a `GenerationConfig`:

```cpp
struct InferenceRequest
{
    std::string prompt{};
    GenerationConfig generation_config{};
};
```

The nested configuration is composition: an inference request *has a* generation configuration. The settings are not copied into separate request fields, so there is one definition of each generation parameter.

### `InferenceResponse`

`InferenceResponse` describes the information a future inference component may return:

```cpp
struct InferenceResponse
{
    std::string generated_text{};
    std::size_t prompt_tokens{0};
    std::size_t generated_tokens{0};
};
```

Token counts are represented as non-negative sizes. The structure does not imply that generation has happened; this Phase does not construct a fake response or populate these fields with simulated results.

## Type choices and trade-offs

- **`std::string` for names and text:** model names, prompts, and generated text are textual values. A filesystem path is different from ordinary text, so `model_path` uses `std::filesystem::path` (available in C++17).
- **`std::size_t` for sizes and counts:** context capacity, maximum new tokens, and token counts are non-negative quantities used as sizes. `std::size_t` is the standard unsigned type used for object sizes and container indexing. An `int` could represent them, but it allows negative values and may be narrower than `std::size_t` on some platforms. A later API boundary may still need range checks when converting from user-provided signed values.
- **`double` for temperature and top-p:** both are fractional settings. `double` provides more precision than `float` and is a natural default for configurable numeric values. A specific inference runtime may ultimately use `float`; conversion can occur at that integration boundary if required.
- **`std::size_t` for token counts:** counts cannot be negative. The values are counts rather than identifiers or elapsed-time measurements.

No field is made a raw pointer or manually managed allocation. The standard-library value types own their data and have straightforward copy and lifetime behavior.

## Why `struct` instead of `class`

These types are simple collections of related values with no invariants or behavior to protect yet. A `struct` communicates that the fields are public data. A `class` is preferable when an object must enforce invariants, hide implementation details, control mutation, or manage resources through a defined interface. Using a class now would add accessors and constructors without solving a current requirement.

The structures use default member initializers and remain aggregates in C++17 because they do not define user-provided constructors or private data members. They can therefore be default-initialized or aggregate-initialized. Aggregate initialization is concise, but its positional nature means callers must follow the field order. Default member initializers make ordinary default construction safe and readable.

## Composition instead of duplicated fields

`InferenceRequest` contains a `GenerationConfig` member. This makes the relationship explicit and lets every request reuse the same generation-settings definition. Duplicating `max_new_tokens`, `temperature`, and `top_p` in each request would create multiple places to maintain if the configuration changed. Composition also makes a function signature such as `runInference(const InferenceRequest& request)` communicate the complete request as one value.

## Namespace and header organization

The `workbench` namespace gives project types a clear scope and reduces the chance of name collisions with unrelated libraries or application code. Callers can refer to `workbench::ModelConfig`, for example, without placing generic names such as `ModelConfig` in the global namespace.

The four structures are closely related, contain data only, and have no out-of-line implementation. A single cohesive header, `include/workbench/models.hpp`, keeps the data contracts together without creating four tiny headers or an unnecessary `.cpp` file. The header uses an include guard and includes the standard headers required by its public field types. CMake adds the `include` directory to this executable's private include path and lists the header as part of the target for project visibility.

## Const references in read-only functions

The status display receives configurations as `const` references:

```cpp
void printStatus(
    const workbench::ModelConfig& model_config,
    const workbench::GenerationConfig& generation_config);
```

A reference avoids making a copy of the object, and `const` documents that the function must not modify it. This is especially useful for larger structures containing strings and other owned data. For tiny scalar values, pass-by-value can be equally or more efficient; the choice depends on the type and purpose. Here, references also make the read-only relationship clear.

## Integration with the existing CLI

`runApplication()` creates default `ModelConfig` and `GenerationConfig` values and passes them by const reference through `handleCommand()` to `printStatus()`. The `status` command displays whether a model is configured, the default context size, and the illustrative generation settings. Since the model name and path are empty by default, the output correctly reports that no model is configured. The inference backend remains unintegrated.

The `help` and `exit` commands, case-insensitive command normalization, whitespace trimming, empty-input behavior, unknown-command message, and EOF handling remain part of the existing CLI flow.

## Keeping data separate from behavior

The structures do not read terminal input, print output, validate command names, access model files, tokenize prompts, generate text, allocate GPU memory, or launch kernels. Those responsibilities belong to application and runtime components. Keeping data contracts passive makes them reusable and avoids coupling simple values to a particular interface or backend.

## Validation and runtime behavior are deferred deliberately

This milestone establishes field names and types, not a complete validation policy. For example, the data model does not reject a zero context size, negative-like values converted from user input, a temperature outside a chosen range, or a non-existent model path. Those checks should be added where inputs enter the system and where the selected runtime's actual constraints are known. Adding speculative rules now could incorrectly assume that all models and runtimes share the same limits.

Likewise, model-file existence checks and inference-response creation belong to future model-loading and inference workflows. No model loading or inference is simulated in this Phase.

## Alternatives considered

- **One header per structure:** this can help when models grow or have different ownership and dependencies. Four small, tightly related data-only types do not yet justify the extra files.
- **Separate `.cpp` implementation:** no functions or non-inline behavior need implementation, so a source file would be empty overhead.
- **Classes with constructors and accessors:** these become useful when invariants or encapsulation are required. Public aggregate structs are simpler for these passive data contracts.
- **`int` for counts:** familiar and convenient, but signed values can be negative. `std::size_t` better communicates non-negative sizes; boundaries that parse signed text should validate before conversion.
- **`float` for generation settings:** some inference engines use single precision. `double` is retained in the application-facing configuration until a specific runtime interface requires conversion.
- **A serialization or validation library:** external packages are unnecessary for four in-memory structures. Serialization and schema validation can be selected when persistence or external configuration formats become requirements.

## Extension path

Later Phases can add validation functions, model-loading components, runtime adapters, resource ownership, and inference execution when those requirements arrive. The data models can then be passed between components without moving CLI behavior into the structures. The existing CMake target, C++17 baseline, and interactive commands remain the project's foundation.
