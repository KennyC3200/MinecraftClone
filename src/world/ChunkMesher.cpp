#include "world/ChunkMesher.hpp"
#include "world/Block.hpp"

#include <array>
#include <cstdint>

namespace mcc {

namespace {

// The face's two counter-clockwise triangles
constexpr std::array<std::uint32_t, 6> QUAD_INDICES = {
	2, 0, 1,
	2, 1, 3
};

std::int32_t PackVertex(const glm::ivec3& pos, Face face, int corner, int tile) {
	return static_cast<std::uint32_t>(pos.x) 
		| static_cast<std::uint32_t>(pos.y) 	<< 5
		| static_cast<std::uint32_t>(pos.z) 	<< 10
		| static_cast<std::uint32_t>(face) 		<< 15
		| static_cast<std::uint32_t>(corner) 	<< 18
		| static_cast<std::uint32_t>(tile) 		<< 20;
}

}

void AppendFace(MeshData& data, const glm::ivec3& block_pos, Face face, int tile) {
	const FaceDef& face_vertices = FACES[static_cast<std::size_t>(face)];

	// Append indices to draw two triangles (6 total)
	std::size_t offset = data.m_vertices.size();
	for (std::uint32_t idx : QUAD_INDICES) {
		data.m_indices.push_back(offset + idx);
	}

	// Append packed vertices
	for (std::size_t corner = 0; corner < face_vertices.m_corners.size(); corner++) {
		glm::ivec3 corner_pos = block_pos + glm::ivec3(face_vertices.m_corners[corner]);
		data.m_vertices.push_back(PackVertex(corner_pos, face, corner, tile));
	}
}

MeshData BuildChunkMesh(const Chunk& chunk) {
	MeshData mesh;

	// Optimized loop order for cache
	for (std::size_t z = 0; z < Chunk::SIZE; z++) {
		for (std::size_t y = 0; y < Chunk::SIZE; y++) {
			for (std::size_t x = 0; x < Chunk::SIZE; x++) {
				glm::ivec3 pos = { x, y, z };
				BlockId block_id = chunk.GetBlock({x, y, z});
				BlockInfo block_info = GetBlockInfo(block_id);

				// If block is not solid
				if (!block_info.m_solid) continue;

				// Build faces
				for (std::size_t f = 0; f < static_cast<std::size_t>(Face::Count); f++) {
					glm::ivec3 neighbour = pos + FACES[f].m_normal;

					// Don't draw face if neighbouring block is solid
					if (GetBlockInfo(chunk.GetBlock(neighbour)).m_solid) continue;

					Face face = static_cast<Face>(f);
					AppendFace(mesh, {x, y, z}, face, block_info.TileFor(face));
				}
			}
		}
	}

	return mesh;
}

}
