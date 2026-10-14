#pragma once

#include "world/Chunk.hpp"

#include <memory>

namespace mcc {

int SurfaceHeight(int x, int z);
std::unique_ptr<Chunk> GenerateChunk(const glm::ivec3& chunk_pos);

}