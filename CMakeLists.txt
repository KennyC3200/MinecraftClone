cmake_minimum_required(VERSION 3.25)

project(MinecraftClone
    VERSION 0.1.0
    DESCRIPTION "A Minecraft clone written in C++"
    LANGUAGES C CXX
)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generates compile_commands.json for VS Code IntelliSense / clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to Debug for single-config generators when no build type is given
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# Dependencies (OpenGL, glad, GLFW, glm, stb) - see external/CMakeLists.txt
# ---------------------------------------------------------------------------
add_subdirectory(external)

# ---------------------------------------------------------------------------
# Sources
# Headers and sources live side by side under src/, grouped by feature folder
# (e.g. src/world/Chunk.h + src/world/Chunk.cpp). New files are picked up
# automatically on the next build thanks to CONFIGURE_DEPENDS.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE MC_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
)

add_executable(MinecraftClone ${MC_SOURCES})

target_include_directories(MinecraftClone PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(MinecraftClone PRIVATE
    glad
    glfw
    glm::glm
    stb
)

target_compile_definitions(MinecraftClone PRIVATE
    MC_VERSION="${PROJECT_VERSION}"
    GLFW_INCLUDE_NONE       # glad provides the OpenGL headers, not GLFW
    GLM_ENABLE_EXPERIMENTAL # For hash functions
)

# ---------------------------------------------------------------------------
# Compiler warnings
# ---------------------------------------------------------------------------
target_compile_options(MinecraftClone PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8 /shadow>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wshadow>
)

# ---------------------------------------------------------------------------
# Output layout: build/<preset>/bin/MinecraftClone[.exe]
# ---------------------------------------------------------------------------
set_target_properties(MinecraftClone PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Make MinecraftClone the startup project if a Visual Studio generator is used
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MinecraftClone)

# ---------------------------------------------------------------------------
# Copy assets next to the executable on every build
# A custom target with no output file is always out of date, so this also runs
# when only assets changed. (A POST_BUILD step would only run when the
# executable is relinked, leaving edited shaders stale.)
# ---------------------------------------------------------------------------
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:MinecraftClone>/assets"
    COMMENT "Copying assets to output directory"
)
add_dependencies(MinecraftClone copy_assets)
