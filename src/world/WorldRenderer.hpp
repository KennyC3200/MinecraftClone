#pragma once

#include "graphics/Shader.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Texture.hpp"
#include "world/World.hpp"
#include "core/ThreadPool.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <cstddef>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>

namespace mcc {

// Timing of WorldRenderer::Update, summarised over the last second. Filled in by the profiler in
// WorldRenderer.cpp and shown by the debug overlay
struct WorldRendererStats {
	struct Phase {
		const char* m_name = "";
		double m_avg_ms = 0.0;
		double m_max_ms = 0.0;
	};

	std::vector<Phase> m_phases;
	Phase m_total{ "total" };

	std::size_t m_frames = 0;
	std::size_t m_dirty_cnt = 0;
	std::size_t m_ready_max = 0;
	std::size_t m_in_flight_cnt = 0;
};

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

	// True while enough chunks are waiting to be meshed that World should pause generating more
	bool IsBacklogged() const { return m_ready.size() >= MAX_READY_BACKLOG; }

	// Update's timing over the last full second (empty until the first second has passed)
	const WorldRendererStats& Stats() const { return m_stats; }

private:
	WorldRendererStats m_stats;

	MapType m_chunk_meshes;
	Shader m_shader;
	Texture m_block_atlas;

	GLint m_chunk_uniform_loc;

	bool NeighboursReady(const World& world, const glm::ivec3& chunk_pos) const;

	// Keep track of all dirty chunks through frames
	std::unordered_set<glm::ivec3> m_dirty;

	struct ReadyChunk {
		int m_dist_squared;
		glm::ivec3 m_pos;
	};

	// Bc m_ready is a min_heap, x > y means that x has a lower priority
	// so the smallest distance ends up at the top
	struct ReadyChunkFarther {
		bool operator()(const ReadyChunk& x, const ReadyChunk& y) const {
			return x.m_dist_squared > y.m_dist_squared;
		}
	};

	std::priority_queue<ReadyChunk, std::vector<ReadyChunk>, ReadyChunkFarther> m_ready;

	static constexpr double MESH_FRAMETIME_MS = 4.0;
	static constexpr int MAX_UPLOADS_PER_FRAME = 8;

	// Counts stale heap entries too, so it is approximate, but that only makes World pause a bit early
	static constexpr std::size_t MAX_READY_BACKLOG = 2048;

	// Meshes of unloaded chunks, reused for new chunks instead of deleting and recreating their GL
	// objects. Spares beyond MAX_SPARE_MESHES are deleted a few per frame, so a big unload can't spike
	std::vector<std::unique_ptr<Mesh>> m_spare_meshes;
	static constexpr std::size_t MAX_SPARE_MESHES = 1024;
	static constexpr std::size_t MAX_MESH_DELETES_PER_FRAME = 32;

	// Moves the chunk's mesh (if any) into m_spare_meshes and removes it from m_chunk_meshes
	void ReleaseMesh(const glm::ivec3& chunk_pos);

	// Returns a spare mesh if there is one, otherwise a new one
	std::unique_ptr<Mesh> AcquireMesh();

	// Store the mesh data once the threads are done
	struct InFlightMeshData {
		glm::ivec3 m_pos;
		std::future<MeshData> m_data;
	};
	std::vector<InFlightMeshData> m_in_flight_jobs;
	std::unordered_set<glm::ivec3> m_in_flight_positions;
};

}