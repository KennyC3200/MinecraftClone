#pragma once

#include "graphics/Mesh.hpp"
#include "world/Chunk.hpp"
#include "world/Face.hpp"

#include <glm/glm.hpp>

namespace mcc {

// Appends one block face (4 vertices, 6 indices) to `data`. `block_pos` is the block's position in
// chunk-local coordinates. `tile` is the face's atlas tile (from BlockInfo::TileFor)
void AppendFace(MeshData& data, const glm::ivec3& block_pos, Face face, int tile);

// Builds a chunk's mesh: for every solid block, adds each face whose neighbour isn't solid.
MeshData BuildChunkMesh(const Chunk& chunk);

}