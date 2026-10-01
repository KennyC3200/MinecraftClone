#include "world/ChunkMesher.hpp"
#include "world/Block.hpp"

#include <array>
#include <cstdint>

namespace mcc {

namespace {

constexpr int ATLAS_TILES_PER_ROW = 16;

// Every FaceDef lists its corners in the same order, so these apply to every face.
constexpr std::array<glm::vec2, 4> CORNER_UVS = {
	glm::vec2(0, 0),
	glm::vec2(1, 0),
	glm::vec2(0, 1),
	glm::vec2(1, 1)
};

// The face's two counter-clockwise triangles
constexpr std::array<std::uint32_t, 6> QUAD_INDICES = {
	2, 0, 1,
	2, 1, 3
};

// Convert from tile's local uv to the uv on the atlas
glm::vec2 AtlasUV(int tile, const glm::vec2& local_uv) {
	int row = tile / ATLAS_TILES_PER_ROW;
	int col = tile % ATLAS_TILES_PER_ROW;
	return glm::vec2(
		(col + local_uv.x) / static_cast<float>(ATLAS_TILES_PER_ROW), 							// u
		(ATLAS_TILES_PER_ROW - 1 - row + local_uv.y) / static_cast<float>(ATLAS_TILES_PER_ROW)	// v
	);
}

}

void AppendFace(MeshData& data, const glm::ivec3& block_pos, Face face, int tile) {
	const FaceDef& face_vertices = FACES[static_cast<std::size_t>(face)];

	// Append indices to draw two triangles (6 total)
	std::size_t offset = data.m_vertices.size();
	for (std::uint32_t idx : QUAD_INDICES) {
		data.m_indices.push_back(offset + idx);
	}

	// Append vertices to the data.m_vertices
	// Vertex: position (3 elements), uv (2 elements)
	for (std::size_t i = 0; i < face_vertices.m_corners.size(); i++) {
		data.m_vertices.emplace_back(
			glm::vec3(block_pos) + face_vertices.m_corners[i],
			AtlasUV(tile, CORNER_UVS[i])
		);
	}
}

MeshData BuildChunkMesh(const Chunk& chunk) {
	MeshData mesh;

	// Optimized loop order for cache
	for (std::size_t z = 0; z < Chunk::SIZE; z++) {
		for (std::size_t y = 0; y < Chunk::SIZE; y++) {
			for (std::size_t x = 0; x < Chunk::SIZE; x++) {
				BlockId block_id = chunk.GetBlock({x, y, z});
				BlockInfo block_info = GetBlockInfo(block_id);
				AppendFace(mesh, {x, y, z}, Face::North, block_info.TileFor(Face::North));
				AppendFace(mesh, {x, y, z}, Face::South, block_info.TileFor(Face::South));
				AppendFace(mesh, {x, y, z}, Face::East, block_info.TileFor(Face::East));
				AppendFace(mesh, {x, y, z}, Face::West, block_info.TileFor(Face::West));
				AppendFace(mesh, {x, y, z}, Face::Up, block_info.TileFor(Face::Up));
				AppendFace(mesh, {x, y, z}, Face::Down, block_info.TileFor(Face::Down));
			}
		}
	}

	return mesh;
}

}
