#pragma once

#include "graphics/Mesh.hpp"
#include "world/Chunk.hpp"
#include "world/Face.hpp"

#include <glm/glm.hpp>

namespace mcc {

// Appends one block face (4 vertices, 6 indices) to `data`. `block_pos` is the block's position in
// chunk-local coordinates. `tile` is the face's atlas tile (from BlockInfo::TileFor)
void AppendFace(MeshData& data, const glm::ivec3& block_pos, Face face, int tile);

// Fields and ranges
// x: 		0 to 15 + 1 -> 5 bits
// y: 		0 to 15 + 1 -> 5 bits
// z: 		0 to 15 + 1 -> 5 bits
// face: 	0 to 5 		-> 3 bits
// corner: 	0 to 3 		-> 2 bits
// tile: 	0 to 255 	-> 8 bits
// spare
MeshData BuildChunkMesh(const Chunk& chunk);

}