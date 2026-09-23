# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A Minecraft clone in C++20 + OpenGL, built with CMake for Windows 11 and macOS. It is a learning project: the user writes the game code themselves. Default to reviewing and explaining; only write or change files under `src/` when explicitly asked, and when asked for "boilerplate" produce stubs without implementations.

## Build & run

Presets live in `CMakePresets.json`; all use the Ninja generator and build into `build/<preset>/`.

```
cmake --preset debug                  # configure (first time, or after CMakeLists changes)
cmake --build --preset debug          # build
./build/debug/bin/MinecraftClone      # run (.exe on Windows)
```

- Presets: `debug`, `release` (use whatever compiler is on PATH), and Windows-only `msvc-debug` / `msvc-release` (force `cl`; need Visual Studio or a "Developer PowerShell for VS" shell).
- The dev machine has no MSVC installed. Its PATH compiler is GCC 13 (Strawberry/MinGW), so only `debug`/`release` work there.
- The first configure downloads dependencies (needs internet, ~20 s).
- After switching compilers, delete `build/<preset>` (or pass `--fresh`), because CMake caches the compiler.
- There are no tests or linters. Verify changes by building, since warnings (`-Wall -Wextra -Wpedantic` / `/W4`) are on for the game target.

## Build system layout

- **Root `CMakeLists.txt`**
  - Globs `src/**/*.cpp|.h|.hpp` with `CONFIGURE_DEPENDS`, so new source files need no CMake edits.
  - Defines `MC_VERSION` (from `project(VERSION)`) and `GLFW_INCLUDE_NONE` on the target.
  - Copies `assets/` next to the executable after each build.
- **`external/CMakeLists.txt`**: all third-party deps.
  - `glad` is **vendored** in `external/glad`, generated for OpenGL 4.1 core + `GL_KHR_debug`. The regenerate command is in that file.
  - GLFW 3.4, glm 1.0.1 (header-only) and stb are fetched with `FetchContent`, pinned by URL + SHA256.
  - `stb` is an INTERFACE target. Exactly one `.cpp` must `#define STB_IMAGE_IMPLEMENTATION` before including `<stb_image.h>`; none exists yet.
- **VS Code IntelliSense**
  - `.vscode/settings.json` points IntelliSense at CMake Tools / `build/debug/compile_commands.json`.
  - Macros like `MC_VERSION` only resolve there after a configure.

## Platform constraints

- **OpenGL version:** target **OpenGL 4.1 core**, the macOS maximum. Don't use 4.3+ features such as compute shaders, DSA or `glDebugMessageCallback` from core. `GL_KHR_debug` is loaded, but check `GLAD_GL_KHR_debug` at runtime.
- **macOS context:** it needs the `GLFW_OPENGL_FORWARD_COMPAT` hint.
- **Viewport size:** use the framebuffer size (not the window size) for `glViewport`, because they differ on high-DPI displays.
- **Include order:** include `<glad/glad.h>` wherever GL calls are made. Because of `GLFW_INCLUDE_NONE`, GLFW headers may come before glad.

## Code conventions

- Namespace `mcc`. Headers are `.hpp`, next to their `.cpp` in `src/`, grouped into feature subfolders as the project grows. Include root is `src/`.
- Members use the `m_` prefix. Braces go on the same line.
- Resource owners (window, GL objects) are RAII classes:
  - Copies are deleted; moves are `noexcept`. Use `std::exchange` in the move constructor and member-wise swap in move assignment.
  - The destructor must skip moved-from (null/0) handles.
  - Constructor failures `throw std::runtime_error` after cleaning up what was already created. Never `exit()`.
- GL-object wrappers must be destroyed before the `Window`, whose GL context they need. Never make them static or global.
- `int` for window/framebuffer dimensions (it matches the GLFW/GL APIs). `std::size_t` for memory sizes and indices.

## Architecture

File names are PascalCase (`Engine.hpp`, `Window.cpp`). Keep `#include` capitalization exact, since macOS and git can be case-sensitive.

- **`mcc::Engine`** (`src/Engine.hpp/.cpp`) is the top-level owner and will run the main loop in `Run()`.
  - It is meant to be built on by separate game classes.
  - Its members are declared in dependency order. Construction follows declaration order and destruction runs in reverse, so new systems go below `m_window` if they need the GL context.
- **`mcc::GLFWContext`** (`src/GLFWContext.hpp`) is a non-copyable, non-movable guard. It calls `glfwInit` / `glfwTerminate` and is the first member of `Engine`. Nothing else should call `glfwInit` or `glfwTerminate`.
- **`mcc::Window`** (`src/Window.hpp/.cpp`) creates the GLFW window and GL context, loads glad, and sets the framebuffer-resize callback. It assumes GLFW is already initialized.
- **`src/main.cpp`** is being reduced to constructing `Engine` and calling `Run()` inside a try/catch.
