# ---------------------------------------------------------------------------
# Third-party dependencies
#
#   OpenGL - system library (opengl32 on Windows, OpenGL.framework on macOS)
#   glad   - OpenGL 4.1 core loader + GL_KHR_debug, vendored in external/glad
#   GLFW   - window, context and input            (downloaded at configure time)
#   glm    - math (vectors, matrices, transforms) (downloaded at configure time)
#   stb    - stb_image for texture loading        (downloaded at configure time)
#
# OpenGL 4.1 core is the newest version macOS supports, so it is the target
# for both platforms.
# ---------------------------------------------------------------------------
include(FetchContent)

find_package(OpenGL REQUIRED)

# --- glad -------------------------------------------------------------------
# Regenerate with: python -m glad --profile core --api gl=4.1
#   --extensions GL_KHR_debug --generator c --spec gl --out-path external/glad
add_library(glad STATIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c")
target_include_directories(glad SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")
target_link_libraries(glad PUBLIC OpenGL::GL)

# --- GLFW -------------------------------------------------------------------
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

FetchContent_Declare(glfw
    URL      https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
    URL_HASH SHA256=b5ec004b2712fd08e8861dc271428f048775200a2df719ccf575143ba749a3e9
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SYSTEM
)

# --- glm --------------------------------------------------------------------
set(GLM_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)  # header-only

FetchContent_Declare(glm
    URL      https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip
    URL_HASH SHA256=09c5716296787e1f7fcb87b1cbdbf26814ec1288ed6259ccd30d5d9795809fa5
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SYSTEM
)

# --- stb --------------------------------------------------------------------
# stb has no CMake project, so it is only downloaded and wrapped below.
FetchContent_Declare(stb
    URL      https://github.com/nothings/stb/archive/2c980bb59875b0d32144a71867fbdebb2f77cd20.zip
    URL_HASH SHA256=8e59f72b0780690cda64726804269f638a3be77b9d1506ea95f443f7964bccf0
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# glm 1.0.1 asks for CMake 3.6, which CMake 4.x warns about
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)

FetchContent_MakeAvailable(glfw glm stb)

# Header-only target. Exactly one .cpp in the project must
# #define STB_IMAGE_IMPLEMENTATION before #include <stb_image.h>.
add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE "${stb_SOURCE_DIR}")
