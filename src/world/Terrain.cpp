#include "Terrain.hpp"

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

namespace mcc {

int SurfaceHeight(int x, int z) {
	float frequency = 1.0f / 64.0f;
	int amplitude = 16;
	float offset = 0.37f;
	int base_height = 64;
	
	float n = stb_perlin_fbm_noise3(
		x * frequency + offset, offset, z * frequency + offset,
		2.0f, 0.5f, 4);

	return base_height + static_cast<int>(n * amplitude);
}

std::unique_ptr<Chunk> GenerateChunk(const glm::ivec3& chunk_pos) {
	auto chunk = std::make_unique<Chunk>();
	glm::ivec3 origin = chunk_pos * Chunk::SIZE;

	// Iterate one column at a time. Each column, get the perlin noise height
	for (int z = 0; z < Chunk::SIZE; z++) {
		for (int x = 0; x < Chunk::SIZE; x++) {
			int h = SurfaceHeight(x + origin.x, z + origin.z);
			for (int y = 0; y < Chunk::SIZE; y++) {
				int wy = origin.y + y;
				if (wy > h) {
					// The chunks are already set to be air
					continue;
				} else if (wy == h) {
					chunk->SetBlock({ x, y, z }, BlockId::Grass);
				} else if (wy >= h - 3) {
					chunk->SetBlock({ x, y, z }, BlockId::Dirt);
				} else {
					chunk->SetBlock({ x, y, z }, BlockId::Stone);
				}
			}
		}
	}

	return chunk;
}

}