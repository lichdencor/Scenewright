# Tier 1 tests (docs/adr/0010-test-suite-strategy.md): pure C++ logic only —
# no CommonLibSSE-NG, no RE::/SKSE:: types — so this is a deliberately
# separate CMake project from the root one, with its own vcpkg.json. It
# builds natively on Linux, independent of the main plugin's Windows-only
# commonlibsse-ng dependency.
#
# Configure/build/run:
#   cmake -S tests -B tests/build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
#   cmake --build tests/build
#   ctest --test-dir tests/build --output-on-failure

cmake_minimum_required(VERSION 3.21)
project(ScenewrightTests LANGUAGES CXX)

find_package(doctest CONFIG REQUIRED)

add_executable(scenewright_tests
    protocol_smoke_test.cpp
    core_init_test.cpp
    fakes/core_fakes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/core.cpp
)
target_compile_features(scenewright_tests PRIVATE cxx_std_23)
target_include_directories(scenewright_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
)
target_link_libraries(scenewright_tests PRIVATE doctest::doctest)

enable_testing()
include(doctest)
doctest_discover_tests(scenewright_tests)
