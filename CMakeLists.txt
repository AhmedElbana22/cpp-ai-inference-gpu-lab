cmake_minimum_required(VERSION 3.16)

project(CppAIInferenceGpuLab VERSION 0.1.0 LANGUAGES CXX)

include(CTest)

function(configure_workbench_target target_name)
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    if(MSVC)
        target_compile_options(${target_name} PRIVATE
            /W4
            /permissive-
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
        )
    endif()
endfunction()

add_executable(cpp_local_llm_inference_workbench
    src/main.cpp
    src/application.cpp
    src/cli.cpp
    src/validation.cpp
    src/backends/mock_inference_backend.cpp
    include/workbench/models.hpp
    include/workbench/application.hpp
    include/workbench/cli.hpp
    include/workbench/inference_backend.hpp
    include/workbench/mock_inference_backend.hpp
    include/workbench/validation.hpp
)

target_include_directories(cpp_local_llm_inference_workbench PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

configure_workbench_target(cpp_local_llm_inference_workbench)

if(BUILD_TESTING)
    add_executable(workbench_validation_tests
        tests/validation_tests.cpp
        src/validation.cpp
        src/application.cpp
        src/cli.cpp
    )

    target_include_directories(workbench_validation_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    configure_workbench_target(workbench_validation_tests)

    add_test(
        NAME workbench_validation_tests
        COMMAND workbench_validation_tests
    )
endif()
