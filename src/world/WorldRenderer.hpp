#pragma once

#include "graphics/Shader.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Texture.hpp"
#include "world/World.hpp"
#include "core/ThreadPool.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace mcc {

class WorldRenderer final {
public:
	using MapType = std::unordered_map<glm::ivec3, std::unique_ptr<Mesh>>;

	WorldRenderer();

	Mesh* GetChunkMesh(const glm::ivec3& chunk_pos);
	const Mesh* GetChunkMesh(const glm::ivec3& chunk_pos) const;

	void Update(const World& world, const ChunkChanges& changes, ThreadPool& thread_pool);
	void RenderWorld(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj);
	void MeshChunk(const World& world, const glm::ivec3& chunk_pos, ThreadPool& thread_pool);

	void SetDirtyChunks(const glm::ivec3 chunk_pos);

private:
	MapType m_chunk_meshes;
	Shader m_shader;
	Texture m_block_atlas;

	GLint m_chunk_uniform_loc;

	bool NeighboursReady(const World& world, const glm::ivec3& chunk_pos) const;

	std::unordered_set<glm::ivec3> m_dirty;
	static constexpr double MESH_FRAMETIME_MS = 4.0;
	static constexpr int MAX_UPLOADS_PER_FRAME = 8;

	// Store the mesh data once the threads are done
	struct InFlightMeshData {
		glm::ivec3 m_pos;
		std::future<MeshData> m_data;
	};
	std::vector<InFlightMeshData> m_in_flight_jobs;
	std::unordered_set<glm::ivec3> m_in_flight_positions;
};

}