# C++ AI Inference & GPU Acceleration Lab

**Main application:** C++ Local LLM Inference Workbench  
**Current milestone:** Phase 05 - Resource Management, Error Handling & Testing

## Project objective

This repository is being developed incrementally into a C++ application for local large-language-model inference. Planned development includes inference-runtime integration, CPU and GPU execution, benchmarking, and CUDA experiments.

The application has an interactive command-line interface (CLI), reusable application data contracts, and a modular architecture with CLI, application, and inference-backend responsibilities. The CLI supports `help`, `status`, and `exit`. The application depends on the `InferenceBackend` abstraction and receives a `MockInferenceBackend` through dependency injection. Phase 05 adds reusable configuration validation and a dependency-free CTest test executable. **No model is loaded and no real inference runtime, CUDA code, or external library is included at this stage.** Future Phases should extend this repository while preserving its project identity and established engineering decisions.

## Current data models

The data-only structures are declared in `include/workbench/models.hpp` inside the `workbench` namespace:

| Structure | Responsibility |
| --- | --- |
| `ModelConfig` | Model name, filesystem path, and context size. Empty name and path mean no model is configured. |
| `GenerationConfig` | Maximum new tokens, temperature, and top-p settings. Defaults are illustrative and configurable, not universal recommendations. |
| `InferenceRequest` | Prompt plus a composed `GenerationConfig`. |
| `InferenceResponse` | Generated text and prompt/generated token counts for a future inference result. |

Phase 04 adds `InferenceBackend`, an abstract interface for backend identity and runtime readiness, plus `MockInferenceBackend`, a placeholder that reports that real inference is not integrated and not ready. The `Application` coordinates commands through the interface, while `Cli` owns terminal input and output. The mock does not load a model or generate responses.

The structures are passive data contracts. They do not load models, validate runtime-specific settings, tokenize prompts, or perform inference. The existing `status` command uses default `ModelConfig` and `GenerationConfig` values to report that no model is configured, the default context size, and the illustrative generation settings. It does not claim that a model has been loaded or that text has been generated.

Configuration validation is declared in `include/workbench/validation.hpp` and implemented in `src/validation.cpp`. `validateModelConfig()` checks that the model name and path are not empty and that the context size is greater than zero. It deliberately does not require the model file to exist. `validateGenerationConfig()` checks the token count, finite non-negative temperature, and finite top-p value in the interval `(0, 1]`. Both functions return `std::optional<std::string>`: no value means valid, while a message describes the first validation failure.

The validation functions are independent of CLI interaction and inference execution. The default `ModelConfig{}` represents an unconfigured model, so it is expected not to pass model-configuration validation. The application continues to report that no model is configured rather than treating an absent model as a loaded or usable one.

## Prerequisites

- CMake 3.16 or newer.
- A C++ compiler with C++17 support, such as MSVC, GCC, or Clang.
- Git, if you want to initialize and maintain the repository history.

No third-party C++ dependencies are required.

Check the installed tools:

```sh
cmake --version
git --version
```

Check your compiler using the command appropriate to your toolchain:

```sh
# GCC
g++ --version

# Clang
clang++ --version

# MSVC (from a Visual Studio Developer Command Prompt)
cl
```

## Project structure

```text
cpp-ai-inference-gpu-lab/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── include/
│   └── workbench/
│       ├── models.hpp
│       ├── application.hpp
│       ├── cli.hpp
│       ├── inference_backend.hpp
│       ├── mock_inference_backend.hpp
│       └── validation.hpp
├── src/
│   ├── main.cpp
│   ├── application.cpp
│   ├── cli.cpp
│   ├── validation.cpp
│   └── backends/
│       └── mock_inference_backend.cpp
├── tests/
│   └── validation_tests.cpp
└── docs/
    ├── Phase-01-setup.md
    ├── Phase-02-cli.md
    ├── Phase-03-data-modeling.md
    ├── Phase-04-architecture.md
    └── Phase-05-resource-management.md
```

The repository contains only the files needed by the completed Phases. New directories and modules will be introduced when later features require them.

## Configure

Run this command from the project root:

```sh
cmake -S . -B build
```

CMake detects the available toolchain and generates build files for its selected generator. The command is intentionally portable and does not hardcode compiler paths.

## Build

```sh
cmake --build build --config Release
```

The `--config Release` option supports multi-configuration generators such as Visual Studio. It is harmless for common single-configuration generators; those generators can also select a build type during configuration with `-DCMAKE_BUILD_TYPE=Release`.

## Run the tests

Phase 05 registers a small, dependency-free test executable with CTest. From the project root, run:

```sh
cmake -E chdir build ctest -C Release --output-on-failure
```

This works with single- and multi-configuration generators. The tests cover valid and invalid `ModelConfig` and `GenerationConfig` values, including NaN and infinity, and compile a second backend implementation against the existing interface. Test failures produce a non-zero exit code. To omit the test target, configure with `-DBUILD_TESTING=OFF`.

## Run

The executable target is named `cpp_local_llm_inference_workbench` (with `.exe` on Windows). Its exact output directory depends on the CMake generator and platform.

**Windows PowerShell - Visual Studio generator:**

```powershell
.\build\Release\cpp_local_llm_inference_workbench.exe
```

**Windows PowerShell - single-configuration generator (for example, Ninja or MinGW Makefiles):**

```powershell
.\build\cpp_local_llm_inference_workbench.exe
```

**Linux or macOS - common single-configuration generators:**

```sh
./build/cpp_local_llm_inference_workbench
```

For a different generator or a custom configuration, locate the executable under the generated `build` directory and run that file.

## Using the CLI

After launching the application, enter a command at the `>` prompt. Commands are case-insensitive, and leading or trailing whitespace is ignored.

| Command | Description |
| --- | --- |
| `help` | Show available commands and their descriptions. |
| `status` | Display application identity, development stage, model-configuration state, default context size, illustrative generation settings, selected backend name, inference readiness, and real-runtime integration status. |
| `exit` | Exit the application gracefully. |

Empty input is ignored. An unsupported command displays an error and suggests using `help`. If the input stream reaches EOF (for example, when standard input is closed), the application shuts down gracefully.

Example interaction:

```text
Welcome to C++ Local LLM Inference Workbench!

Available commands:
- help
- status
- exit

> help

Available commands:
- help    Show available commands
- status  Display application information
- exit    Exit the application

> status

Application: C++ Local LLM Inference Workbench
Project: C++ AI Inference & GPU Acceleration Lab
Development stage: Phase 05 - Resource Management, Error Handling & Testing
Status: Initialized
Model configured: No
Model name: Not configured
Model path: Not configured
Default context size: 2048 tokens
Illustrative generation defaults:
- max_new_tokens: 128
- temperature: 0.7
- top_p: 0.9
Backend: MockInferenceBackend
Inference readiness: Not ready
Real inference runtime: Not integrated

> exit

Shutting down application...
```

## Initialize Git

Run the following from the project root if this repository has not already been initialized:

```sh
git init
git add CMakeLists.txt README.md .gitignore include src tests docs
git commit -m "Add configuration validation and unit tests"
```

Build output is ignored by `.gitignore`. The distributable project ZIP intentionally excludes the `.git` directory.

## Engineering decisions and Phase documentation

The initial tooling and architecture decisions are documented in [docs/Phase-01-setup.md](docs/Phase-01-setup.md). Phase 02's function responsibilities, input handling, command flow, and design trade-offs are documented in [docs/Phase-02-cli.md](docs/Phase-02-cli.md). Phase 03's data contracts, type choices, composition, const references, and extension trade-offs are documented in [docs/Phase-03-data-modeling.md](docs/Phase-03-data-modeling.md). Phase 04's layer responsibilities, backend abstraction, polymorphism, dependency injection, and alternatives are documented in [docs/Phase-04-architecture.md](docs/Phase-04-architecture.md). Phase 05's lifetime and ownership choices, RAII, smart-pointer trade-offs, configuration validation, error-handling strategy, and CTest setup are documented in [docs/Phase-05-resource-management.md](docs/Phase-05-resource-management.md).

The project retains its CMake-based build, C++17 requirement, compiler warning settings, and original application executable target. The CLI is implemented in `src/cli.cpp`, command coordination is handled by `src/application.cpp`, and `src/main.cpp` is the composition root. The four passive data structures remain in `models.hpp`; validation is implemented separately in `validation.cpp`. The application depends on `InferenceBackend`, not on the mock implementation. CTest runs the standalone validation tests without an external framework. No Python, PyTorch, Transformers, CUDA, llama.cpp, model files, inference libraries, or third-party C++ dependencies are introduced.
