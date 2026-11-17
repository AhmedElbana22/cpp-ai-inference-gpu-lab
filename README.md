# C++ AI Inference & GPU Acceleration Lab

**Main application:** C++ Local LLM Inference Workbench  
**Current milestone:** Phase 01 — Project Setup & Development Environment

## Project objective

This repository will grow incrementally into a C++ application for local large-language-model inference. Planned development includes inference-runtime integration, CPU and GPU execution, benchmarking, and CUDA experiments.

The current Phase establishes the build environment and a small executable only. **No model, inference runtime, CUDA code, or external library is included at this stage.** Future Phases should extend this repository while preserving its project identity and existing engineering decisions.

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
├── src/
│   └── main.cpp
└── docs/
    └── Phase-01-setup.md
```

The repository starts with only the files needed for this milestone. New directories and modules will be introduced when later features require them.

## Configure

Run these commands from the project root:

```sh
cmake -S . -B build
```

CMake detects the available toolchain and generates build files for its selected generator. You can explicitly select a generator when needed, but the default command above is intentionally portable.

## Build

```sh
cmake --build build --config Release
```

The `--config Release` option supports multi-configuration generators such as Visual Studio. It is harmless for common single-configuration generators; those generators can also select a build type during configuration with `-DCMAKE_BUILD_TYPE=Release`.

## Run

The executable is named `cpp_local_llm_inference_workbench` (with `.exe` on Windows). Its exact output directory depends on the CMake generator and platform.

Typical commands after a successful build:

**Windows PowerShell — Visual Studio generator:**

```powershell
.\build\Release\cpp_local_llm_inference_workbench.exe
```

**Windows PowerShell — single-configuration generator (for example, Ninja or MinGW Makefiles):**

```powershell
.\build\cpp_local_llm_inference_workbench.exe
```

**Linux or macOS — common single-configuration generators:**

```sh
./build/cpp_local_llm_inference_workbench
```

For a different generator or a custom configuration, locate the executable under the generated `build` directory and run that file.

## Expected output

```text
===============================================
C++ Local LLM Inference Workbench
Project: C++ AI Inference & GPU Acceleration Lab
Development stage: Phase 01 - Project Setup & Development Environment
Environment status: Initialized
Inference runtime: Not integrated yet
===============================================
```

The program exits with `EXIT_SUCCESS` after printing this status.

## Initialize Git

Run the following from the project root:

```sh
git init
git add CMakeLists.txt README.md .gitignore src/main.cpp docs/Phase-01-setup.md
git commit -m "Initial project setup"
```

Build output is ignored by `.gitignore`. The distributable project ZIP intentionally excludes the `.git` directory.

## Initial engineering decisions

The rationale for the initial tooling and architecture is documented in [docs/Phase-01-setup.md](docs/Phase-01-setup.md). In summary, this milestone uses CMake, requires standard C++17, and builds one executable without introducing future components prematurely.
