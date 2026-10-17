# Minecraft Clone
A Minecraft clone voxel game that is written in c++20 and uses OpenGL 4.1 core. It runs on both Windows and macOS.

## Features
- Infinite procedural terrain generation from a 4-octave fractal Perlin noise
- 32 chunk render distance by default at stable 120 FPS+ at 1080p on Intel Core Ultra 165h
- Multithreading that uses 6 threads total. 6 threads are used to maximize the 6 P-cores my Intel processor
    - 1 main thread
    - 1 thread for GL calls
    - 4 worker threads that are used to generate new chunk data and mesh chunks asynchronously. For generation, it is one 8-chunk column per task and for meshing it is one mesh per task.
- Utilizes the Amanatides & Woo raycasting algorithm for block placement and deletion

## Optimizations
- Vertex attributes are bitwise packed into one 32-bit unsigned integer
- The edited chunk jumps to the front of the meshing queue. When the block sits on a chunk edge, the neighbouring chunk is re-meshed as well.
- Hidden face culling for solid faces directly next to other solid blocks
- Face culling for faces that are blocked by other blocks
- Utilizes frustum culling via the Gribb-Hartmann projection-view matrix each frame
- Utilizes cache friendly loops to maximize cache hits

## World Generation
- Pending chunk columns are sorted by distance, and the meshing queue is a min-heap keyed by distance to the player
- Chunks wait until its 4 neighbours are loaded before meshing. This prevents re-meshing
- Submission of meshing jobs are capped to consume at most 4 ms of frametime to prevent frame spikes
- World generation pauses when >2048 meshes are waiting to be meshed to allow it to catch up

### GPU resources
- Addressing the unloading frametime ms consumption:
  - When a chunk unloads, its VAO/VBO/EBO are kept as spares. A newly loaded chunk reuses them and only refills the buffers, instead of creating new GL objects.
  - At most 1024 spare meshes are kept. Extra ones are deleted at most 32 per frame, so a large unload can't stall a frame.

## Controls
| Key | Action |
| --- | --- |
| W A S D | Move |
| Mouse | Look around |
| Space | Jump (fly up while flying) |
| Left Shift | Fly down |
| Left Ctrl | Sprint |
| F | Toggle flying |
| Left click | Break block |
| Right click | Place the selected block |
| 1 – 9 | Select hotbar slot |
| F3 | Toggle the debug overlay |
| T | Toggle wireframe |
| Esc | Capture / release the mouse cursor |
| Q | Quit |

## Building
Requires CMake 3.25+, Ninja and a C++20 compiler (GCC, Clang or MSVC). The first configure downloads the dependencies, so it needs an internet connection.

```sh
cmake --preset release              # configure
cmake --build --preset release      # build
./build/release/bin/MinecraftClone
```

Run the game from `build/<preset>/bin/`, or from any folder that has the `assets/` directory in it. Assets are loaded by relative path, and they are copied next to the executable on every build.

## Libraries
- [GLFW 3.4](https://www.glfw.org/): window, OpenGL context and input
- [glad](https://github.com/Dav1dde/glad): OpenGL 4.1 core function loader (vendored)
- [glm 1.0.1](https://github.com/g-truc/glm): math
- [stb](https://github.com/nothings/stb): `stb_image` for textures and `stb_perlin` for terrain noise
- [Dear ImGui 1.91.9b](https://github.com/ocornut/imgui): HUD and debug overlay
