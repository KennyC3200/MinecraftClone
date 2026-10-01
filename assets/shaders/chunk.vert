#version 330 core
layout (location = 0) in uint vert;

out vec2 uv_coords;

uniform ivec3 chunk_origin;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    // Unpack the vertex
    uint x      = vert & 31u;             // 5 bits
    uint y      = (vert >> 5u) & 31u;
    uint z      = (vert >> 10u) & 31u;
    uint face   = (vert >> 15u) & 7u;     // 3 bits
    uint corner = (vert >> 18) & 3u;      // 2 bits
    uint tile   = (vert >> 20) & 255u;    // 8 bits

    ivec3 local_pos = ivec3(x, y, z);

    // UV coordinates
    vec2 local_uv = vec2(corner & 1u, corner >> 1u);
    uint row = tile / 16u;
    uint col = tile % 16u;
    vec2 tile_origin = vec2(col, 16u - 1u - row) / 16.0;
    uv_coords = tile_origin + local_uv / 16.0;

    gl_Position = proj * view * model * vec4(chunk_origin + local_pos, 1.0);
}