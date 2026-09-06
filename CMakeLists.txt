# This file configures the "build" of this SKSE plugin
# - how to compile the C++ plugin (in addition to settings found in CMakePresets.json)
# - where to output the resulting .dll file

cmake_minimum_required(VERSION 3.21)

project(Scenewright VERSION 0.0.1 LANGUAGES CXX)

# If you're not using a mod manager, the SKSE plugin can go straight into the
# Skyrim "Data" folder. Set the `SKYRIM_FOLDER` environment variable to your
# Skyrim Special Edition folder to have the build copy it there automatically.
if(DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_FOLDER}/Data")
endif()

# If you're using Mod Organizer 2 or Vortex, set `SKYRIM_MODS_FOLDER` to your
# mods folder instead, and the plugin will land in its own "Scenewright" subfolder.
if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME}")
endif()

# Otherwise, you can set OUTPUT_FOLDER to any place you'd like :)
# set(OUTPUT_FOLDER "C:/path/to/any/folder")

find_package(CommonLibSSE CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ixwebsocket CONFIG REQUIRED)
find_package(SimpleIni CONFIG REQUIRED)

# Explicit file list (no GLOB) — one authoritative, greppable inventory of
# what's compiled, per docs/adr/0009-code-organization-pattern.md. Add each
# new module .cpp here as it lands.
#
# MINIMUM_SKSE_VERSION is spelled out fully (all four dotted components) to
# work around a CommonLibSSE-NG vcpkg port bug: commonlibsse_parse_version()
# relies on CMake's CMAKE_MATCH_<n> globals, which a partial version string
# (its own "0" default) leaves stale from whatever unrelated regex last ran
# during configure — this project's find_package(nlohmann_json)/(ixwebsocket)
# calls are enough to make that manifest as a syntax error in the generated
# plugin-info stub. A fully-specified version matches every capture group
# fresh, sidestepping it regardless of what ran before.
add_commonlibsse_plugin(${PROJECT_NAME} MINIMUM_SKSE_VERSION "0.0.0.0" SOURCES
    src/plugin.cpp
    src/core/core.cpp
    src/core/core_engine.cpp
    src/config/config.cpp
    src/livelink/livelink.cpp
    src/livelink/livelink_server.cpp
    shared/protocol/envelope.cpp
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
target_include_directories(${PROJECT_NAME} PRIVATE src shared)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json ixwebsocket::ixwebsocket SimpleIni::SimpleIni)

# When the SKSE .dll is compiled, automatically copy it into the mods folder.
# Only applies if OUTPUT_FOLDER was set above (directly or via env vars).
if(DEFINED OUTPUT_FOLDER)
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    endif()
endif()
