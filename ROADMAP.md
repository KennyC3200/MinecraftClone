# Roadmap

Each milestone ends in something visible on screen. Finish and verify one before starting the next.

**Workflow per milestone:** write it as raw OpenGL in `Engine` → get it working → extract it into a class → re-run and confirm nothing changed.

## Setup
- [x] CMake project, presets (Ninja / MSVC), cross-platform build
- [x] Libraries: OpenGL 4.1 core, glad, GLFW, glm, stb
- [x] `GLFWContext` (RAII glfwInit/glfwTerminate)
- [x] `Window` (RAII window + GL context)
- [x] `Engine` owning everything, with the main loop in `Run()`

## Rendering basics
- [ ] **0. GL error reporting**: GL errors print to the console
  - `GL_KHR_debug` callback when `GLAD_GL_KHR_debug` is available (Windows)
  - `glGetError` fallback (macOS)
- [x] **1. Triangle**: one coloured triangle on screen
  - Vertex + fragment shader (`#version 410 core`)
  - Extract `Shader` (compile/link errors in the exception message), `VertexBuffer`, `VertexArray`
- [x] **2. Square**: a quad drawn from 4 vertices + 6 indices
  - Extract `IndexBuffer`
- [x] **3. Transforms**: the quad spins
  - Uniforms (`SetMat4`, cached locations), glm model matrix, delta time from `glfwGetTime()`
- [x] **4. 3D cube**: a rotating cube, each face a different colour
  - Perspective projection + view matrix, `glEnable(GL_DEPTH_TEST)`
  - 24 vertices (4 per face) + 36 indices
- [ ] **5. Camera**: fly around the cube with WASD + mouse
  - `Camera` (position, yaw/pitch → view matrix), keyboard/mouse input, cursor capture
- [ ] **6. Textures**: a textured grass block
  - `Texture` with stb_image (one `.cpp` defines `STB_IMAGE_IMPLEMENTATION`), texture coordinates
  - Texture atlas with different top / side / bottom faces

## World
- [ ] **7. Chunk**: a 16×16×16 chunk of blocks drawn as one mesh
  - Block ID array, mesh built from only the faces not hidden by a neighbouring block
  - Back-face culling (`glEnable(GL_CULL_FACE)`)
- [ ] **8. Terrain**: a landscape made of many chunks
  - Chunk neighbours (hide faces on chunk borders), noise-based height map
  - Load/unload chunks around the player
- [ ] **9. Gameplay**: walk, break and place blocks
  - Raycast for block selection, AABB collision, gravity
  - Fixed timestep for physics/game logic

## Design decisions
- **OpenGL 4.1 core only:** the macOS maximum. No compute shaders, no DSA; bind before configuring.
- **RAII for every resource:** move-only classes whose destructors skip moved-from handles. Constructors throw on failure.
- **Member order in `Engine` = dependency order:** GL objects are declared below `m_window` so they are destroyed while the context still exists.
- **Composition over inheritance:** one `Shader` / `Texture` / buffer class, many instances. Different shaders differ in data, not behaviour.
- **Asset paths:** relative paths resolve against the working directory, not the executable. Revisit this once assets are loaded from different launch locations.
