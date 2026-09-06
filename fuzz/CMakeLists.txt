# Tier 3 (docs/adr/0010-test-suite-strategy.md): libFuzzer target for the
# untrusted-peer wire-protocol boundary (docs/adr/0007). Requires Clang —
# libFuzzer (-fsanitize=fuzzer) is a Clang/LLVM feature, not available via
# GCC. Separate CMake project from tests/ and the root plugin, same
# reasoning as tests/: native Linux, no CommonLibSSE-NG dependency.
#
# Configure/build/run:
#   CC=clang CXX=clang++ cmake -S fuzz -B fuzz/build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
#   cmake --build fuzz/build
#   ./fuzz/build/envelope_fuzzer -max_total_time=60

cmake_minimum_required(VERSION 3.21)
project(ScenewrightFuzz LANGUAGES CXX)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "fuzz/ requires Clang (libFuzzer is a Clang/LLVM feature) — configure with CC=clang CXX=clang++")
endif()

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(envelope_fuzzer
    envelope_fuzzer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/protocol/envelope.cpp
)
target_compile_features(envelope_fuzzer PRIVATE cxx_std_23)
target_include_directories(envelope_fuzzer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../shared)
target_link_libraries(envelope_fuzzer PRIVATE nlohmann_json::nlohmann_json)
target_compile_options(envelope_fuzzer PRIVATE -fsanitize=fuzzer,address)
target_link_options(envelope_fuzzer PRIVATE -fsanitize=fuzzer,address)
