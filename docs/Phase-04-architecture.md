# Phase 04 — Application Architecture & Abstraction

## Scope

This Phase extends the existing C++17 workbench without replacing its CLI behavior or Phase 03 data models. The application is separated into three responsibilities: terminal interaction (`Cli`), command coordination (`Application`), and inference-backend capability reporting (`InferenceBackend`). A `MockInferenceBackend` is injected as the current placeholder implementation. It does not load a model or perform inference.

## Why the previous structure needed improvement

In Phase 02 and Phase 03, the command loop, input normalization, command handling, and terminal output lived together in `main.cpp`. That was reasonable while the program was small. As the application gains more responsibilities, keeping all of them in one translation unit makes changes harder to isolate and encourages unrelated code to become coupled.

Phase 04 moves those responsibilities into focused components while retaining the same `help`, `status`, and `exit` commands, case-insensitive command recognition, whitespace handling, empty-line behavior, unknown-command response, and EOF shutdown behavior.

## The three layers

### CLI layer — `Cli`

`Cli` is responsible for communicating with the user through standard input and output. Its methods display the welcome message, help, status, errors, and shutdown messages, and read complete command lines with `std::getline()`.

The CLI does not decide what a command means and does not call an inference runtime. For the status display, it receives the model and generation configurations plus simple backend information from the application. It does not need to know the concrete backend type.

### Application layer — `Application`

`Application` owns the workflow. It displays the welcome message, repeatedly asks the CLI for input, normalizes commands, dispatches `help`, `status`, and `exit`, and coordinates the status information supplied by the backend interface.

The class receives `Cli`, `InferenceBackend`, `ModelConfig`, and `GenerationConfig` through its constructor. It stores non-owning references because those objects are created in `main()` and outlive the application object. The application does not create a `MockInferenceBackend` internally.

### Backend layer — `InferenceBackend` and `MockInferenceBackend`

`InferenceBackend` describes the minimal backend information currently needed by the application. `MockInferenceBackend` is one concrete implementation. It identifies itself as `MockInferenceBackend`, reports that inference is not ready, and reports that no real inference runtime is integrated.

The mock is an architectural placeholder, not a simulator. It has no model-loading or generation behavior and never fabricates an `InferenceResponse`.

## Architecture diagram

```text
                         main()  [composition root]
                          |       creates concrete objects
              +-----------+-----------+----------------+
              |                       |                |
              v                       v                v
            Cli                MockInferenceBackend  ModelConfig /
              ^                       |               GenerationConfig
              |                       | implements
              |                       v
              |                InferenceBackend
              |                       ^
              |                       |
              +------ injected -------+
                      into
                        |
                        v
                  Application
                        |
             coordinates command flow
                        |
             +----------+----------+
             |          |          |
             v          v          v
            help      status      exit
             |          |
             +---- Cli output -----+
                        |
                        +-- reads backend information through
                            InferenceBackend, never through the
                            MockInferenceBackend concrete type
```

The relationships can also be summarized as:

- `main()` constructs `Cli`, the selected backend, and the Phase 03 configuration values.
- `Application` receives references to those dependencies.
- `Application` calls the abstract `InferenceBackend` methods.
- `Cli` is passed display-ready values and handles terminal I/O.
- `MockInferenceBackend` implements the interface but no inference operations.

## What abstraction means here

Abstraction means the application uses a small description of what a backend provides instead of depending on the details of one particular backend library. The application needs to know the backend's name, whether inference can currently be performed, and whether a real runtime is integrated. It does not need to know how a future runtime loads model files or allocates CPU/GPU memory.

The `InferenceBackend` class is an interface: it describes operations that concrete backend classes must provide. It does not contain the implementation of those operations.

```cpp
class InferenceBackend
{
public:
    virtual ~InferenceBackend() = default;

    virtual std::string name() const = 0;
    virtual bool isInferenceReady() const noexcept = 0;
    virtual bool isRuntimeIntegrated() const noexcept = 0;
};
```

The `= 0` syntax makes each function pure virtual. A class that does not implement all inherited pure virtual functions remains abstract and cannot be instantiated directly.

The interface exposes three small queries. The first two provide the requested backend identity and inference readiness. The third reports runtime integration separately. This distinction matters: a real runtime could be integrated while no model has been loaded yet, so "runtime integrated" and "inference ready" are not necessarily the same state.

## Why the interface has a virtual destructor

A base class intended for polymorphic use should have a virtual destructor. If a derived backend is ever destroyed through a pointer to `InferenceBackend`, virtual destruction ensures that the derived destructor is called as part of object destruction. The current application uses non-owning references and does not delete a backend through the interface, but declaring the virtual destructor makes the interface safe for that common future use.

## Runtime polymorphism

`Application` stores an `InferenceBackend&`. When it calls `backend_.name()` or `backend_.isInferenceReady()`, C++ dispatches the call to the override belonging to the actual object supplied at runtime. In this Phase, the actual object is `MockInferenceBackend`.

A future backend can implement the same functions. The application can then receive that implementation without changing its command-processing logic, provided the new class honors the interface contract.

For example, a different implementation can satisfy the same interface without inheriting from or including the mock backend:

```cpp
class AlternativeBackend final : public workbench::InferenceBackend
{
public:
    std::string name() const override { return "AlternativeBackend"; }
    bool isInferenceReady() const noexcept override { return false; }
    bool isRuntimeIntegrated() const noexcept override { return true; }
};
```

This small example deliberately reports that a runtime is integrated while inference is not ready, illustrating why those two status questions are separate. It is an interface demonstration only, not an inference implementation.

## Dependency injection and the composition root

Dependency injection means that a component receives objects it needs from outside rather than constructing specific dependencies internally. `Application` receives references to the CLI, backend interface, and configuration objects in its constructor.

The `main()` function is the composition root: the place where concrete application objects are created and connected.

```cpp
workbench::Cli cli;
workbench::MockInferenceBackend backend;
const workbench::ModelConfig model_config{};
const workbench::GenerationConfig generation_config{};

workbench::Application application(
    cli, backend, model_config, generation_config);
application.run();
```

The references are non-owning. The objects are declared before `Application`, so they remain alive while the application uses them. This is simple and sufficient here; smart-pointer ownership or a dependency-injection framework would add complexity without solving a current ownership problem.

## Why `Application` depends on the interface

The application header includes `inference_backend.hpp`, not `mock_inference_backend.hpp`. This avoids coupling command-processing logic to one backend implementation. The concrete mock is only included by `main.cpp`, where the implementation is selected and injected.

This follows the practical form of the Open/Closed Principle: a new backend can be added by implementing the interface and selecting it in the composition root. The application command loop need not be rewritten merely to change the backend implementation.

## Why a mock backend is used

The mock makes the architecture executable and allows the `status` command to show which backend has been selected. It also proves that the application can call a backend through an interface. It deliberately returns `false` for both inference readiness and real-runtime integration.

The interface does not yet declare model loading or generation methods. Those methods should be shaped when the real runtime and its lifecycle are understood. Designing a full runtime API now would be speculative: model-loading parameters, resource ownership, cancellation, streaming output, batching, error reporting, and runtime-specific capabilities have not yet been selected. Adding unused virtual functions would freeze assumptions prematurely.

## Why the Phase 03 data models remain separate

`ModelConfig`, `GenerationConfig`, `InferenceRequest`, and `InferenceResponse` remain in `include/workbench/models.hpp` unchanged. They describe application data. They do not perform terminal I/O, command dispatch, model loading, or inference. `Application` and `Cli` consume the configuration values through const references for read-only display.

## Alternatives and trade-offs

### Monolithic implementation

Keeping all logic in `main.cpp` has very little setup cost and was appropriate for the first CLI. As responsibilities increase, however, the file becomes a single place for terminal I/O, command decisions, configuration, and runtime integration. The new files establish boundaries before those concerns grow further.

### Procedural functions

A set of free functions can separate responsibilities without classes and may be ideal for a very small program. Here, `Cli` and `Application` naturally group related operations and hold injected dependencies. The classes are kept small and do not introduce inheritance except for the backend interface.

### Direct dependency on `MockInferenceBackend`

This is the simplest short-term option, but it would make `Application` depend on the current placeholder. A future backend swap would require changing application code. Depending on `InferenceBackend` costs one small interface and keeps backend selection outside the application.

### Command-pattern architecture

A command-pattern design could represent each command as a separate object with a common execution interface. That can help when commands become numerous, independently configured, or dynamically registered. Three simple commands do not justify that additional class structure yet; conditional dispatch remains easy to understand and maintain.

### Owning pointers or dependency-injection frameworks

Smart pointers are appropriate when ownership is dynamic or shared, and frameworks may help in large applications. This Phase has a single composition root and clearly scoped objects, so non-owning references are more direct. Ownership can be revisited when backend lifetime or asynchronous work requires it.

## Build and run

From the project root:

```sh
cmake -S . -B build
cmake --build build --config Release
```

Run the generated `cpp_local_llm_inference_workbench` executable from the build directory. For example, on a common Linux/Ninja build:

```sh
./build/cpp_local_llm_inference_workbench
```

The existing CMake minimum version, C++17 requirement, executable target, compiler warning options, and include directory are preserved.

## Extension path

A later Phase can introduce a real backend adapter and select it in `main()` without making `Application` include its concrete header. Model loading, request execution, runtime errors, resource ownership, inference generation, CUDA, and performance work remain deferred until their requirements are introduced. The current mock must continue to report that inference is not ready until a real implementation is integrated and configured.
