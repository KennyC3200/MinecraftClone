#include "WorldRenderer.hpp"
#include "ChunkMesher.hpp"
#include "graphics/Frustum.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <unordered_set>

namespace mcc {

using Clock = std::chrono::steady_clock;
using ms = std::chrono::duration<double, std::milli>;

namespace {

// Debug timing for WorldRenderer::Update. Every phase is timed each frame, and once per second the
// average and worst time of each phase over that second is published to WorldRendererStats, which
// the debug overlay shows. Set to false to turn it off
constexpr bool PROFILE_UPDATE = true;

enum class Phase : std::size_t { Unload, MarkDirty, Collect, Submit, Upload, Count };

constexpr std::size_t PHASE_CNT = static_cast<std::size_t>(Phase::Count);
constexpr std::array<const char*, PHASE_CNT> PHASE_NAMES = {
	"unload", "mark dirty", "collect ready", "submit jobs", "upload"
};

class UpdateProfiler final {
public:
	// Call at the start of Update
	void Begin() { m_last = Clock::now(); }

	// Call at the end of each phase. Times the phase since the previous Lap() or Begin()
	void Lap(Phase phase) {
		Clock::time_point now = Clock::now();
		m_frame[static_cast<std::size_t>(phase)] = ms(now - m_last).count();
		m_last = now;
	}

	// Call at the end of Update. Publishes to stats and resets once the window has run for a second
	void End(
		std::size_t dirty_cnt,
		std::size_t ready_cnt,
		std::size_t in_flight_cnt,
		WorldRendererStats& stats)
	{
		if (!PROFILE_UPDATE) return;

		double total = 0.0;
		for (std::size_t i = 0; i < PHASE_CNT; i++) {
			m_sum[i] += m_frame[i];
			m_max[i] = std::max(m_max[i], m_frame[i]);
			total += m_frame[i];
		}
		m_total_sum += total;
		m_total_max = std::max(m_total_max, total);
		m_ready_max = std::max(m_ready_max, ready_cnt);
		m_frames++;

		if (Clock::now() - m_window_start < std::chrono::seconds(1)) return;

		Publish(dirty_cnt, in_flight_cnt, stats);
		*this = UpdateProfiler();
	}

private:
	void Publish(std::size_t dirty_cnt, std::size_t in_flight_cnt, WorldRendererStats& stats) const {
		double frames = static_cast<double>(m_frames);

		stats.m_phases.resize(PHASE_CNT);
		for (std::size_t i = 0; i < PHASE_CNT; i++) {
			stats.m_phases[i] = { PHASE_NAMES[i], m_sum[i] / frames, m_max[i] };
		}
		stats.m_total = { "total", m_total_sum / frames, m_total_max };

		stats.m_frames = m_frames;
		stats.m_dirty_cnt = dirty_cnt;
		stats.m_ready_max = m_ready_max;
		stats.m_in_flight_cnt = in_flight_cnt;
	}

	Clock::time_point m_window_start = Clock::now();
	Clock::time_point m_last;

	std::array<double, PHASE_CNT> m_frame{};
	std::array<double, PHASE_CNT> m_sum{};
	std::array<double, PHASE_CNT> m_max{};
	double m_total_sum = 0.0;
	double m_total_max = 0.0;
	std::size_t m_ready_max = 0;
	std::size_t m_frames = 0;
};

}

WorldRenderer::WorldRenderer() 
	: m_shader("assets/shaders/chunk.vert", "assets/shaders/chunk.frag")
	, m_block_atlas("assets/textures/blocks.png", GL_RGBA, GL_RGBA) 
{
	m_chunk_uniform_loc = m_shader.UniformLoc("chunk_origin");
}

Mesh* WorldRenderer::GetChunkMesh(const glm::ivec3& chunk_pos) {
	// `it` returns an iterator that contains the ivec3 and unique_ptr
	// So use it->second to access the unique_ptr
	auto it = m_chunk_meshes.find(chunk_pos);
	return it != m_chunk_meshes.end() ? it->second.get() : nullptr;
}

const Mesh* WorldRenderer::GetChunkMesh(const glm::ivec3& chunk_pos) const {
	// `it` returns an iterator that contains the ivec3 and unique_ptr
	// So use it->second to access the unique_ptr
	auto it = m_chunk_meshes.find(chunk_pos);
	return it != m_chunk_meshes.end() ? it->second.get() : nullptr;
}

void WorldRenderer::Update(
	const World& world, 
	const ChunkChanges& changes, 
	ThreadPool& thread_pool)
{
	// Lambda for calculating the distance squared
	glm::ivec3 center = world.GetCenter();
	auto dist_squared = [&](const glm::ivec3& p) {
		int dx = p.x - center.x;
		int dz = p.z - center.z;
		return dx * dx + dz * dz;
	};

	// Lambda for trying to add itself and neighbours
	auto try_queue = [&](const glm::ivec3& pos) {
		if (m_dirty.contains(pos) && NeighboursReady(world, pos)) {
			m_ready.emplace(dist_squared(pos), pos);
		}
	};

	static UpdateProfiler profiler;
	profiler.Begin();

	// Erase the unloaded chunks from the meshes, keeping their meshes as spares for new chunks
	for (const glm::ivec3& pos : changes.m_unloaded) {
		ReleaseMesh(pos);
		m_dirty.erase(pos);
	}

	// Delete the spares beyond the limit, but only a few per frame since each is 3 GL deletes
	for (
		std::size_t i = 0; 
		i < MAX_MESH_DELETES_PER_FRAME && m_spare_meshes.size() > MAX_SPARE_MESHES; 
		i++) 
	{
		m_spare_meshes.pop_back();
	}
	profiler.Lap(Phase::Unload);

	// Add the chunks to be loaded to the set
	for (const glm::ivec3& pos : changes.m_loaded) {
		m_dirty.insert(pos);
		for (const FaceDef& face : FACES) {
			glm::ivec3 n = pos + face.m_normal;
			if (world.GetChunk(n)) {
				m_dirty.insert(n);
			}
		}
	}
	profiler.Lap(Phase::MarkDirty);

	if (changes.m_center_moved) {
		// The heap stored distances are out of date, hence need to rebuild/re-sort the heap
		std::vector<ReadyChunk> ready_chunks;
		for (const glm::ivec3& pos : m_dirty) {
			if (NeighboursReady(world, pos)) {
				ready_chunks.emplace_back(dist_squared(pos), pos);
			}
		}
		m_ready = decltype(m_ready)(ReadyChunkFarther{}, std::move(ready_chunks));
	} else {
		// Only a loaded chunk and its 4 neighbours can have become ready
		for (const glm::ivec3& pos : changes.m_loaded) {
			try_queue(pos);
			try_queue(pos + glm::ivec3(DirVec(Dir::North)));
			try_queue(pos + glm::ivec3(DirVec(Dir::South)));
			try_queue(pos + glm::ivec3(DirVec(Dir::East)));
			try_queue(pos + glm::ivec3(DirVec(Dir::West)));
		}
	}
	profiler.Lap(Phase::Collect);

	std::size_t max_in_flights = thread_pool.Size() * 2;
	auto start = Clock::now();
	while (!m_ready.empty()) {
		if (ms(Clock::now() - start).count() > MESH_FRAMETIME_MS) break;
		if (m_in_flight_positions.size() >= max_in_flights) break;

		ReadyChunk ready = m_ready.top();
		m_ready.pop();

		// Already meshed, unloaded, or duplicate entry
		if (!m_dirty.contains(ready.m_pos)) continue;

		// Still being meshed
		if (m_in_flight_positions.contains(ready.m_pos)) continue;

		MeshChunk(world, ready.m_pos, thread_pool);
		m_dirty.erase(ready.m_pos);
	}
	profiler.Lap(Phase::Submit);

	int uploads = 0;

	// Mesh the in mesh data once it is complete
	for (auto it = m_in_flight_jobs.begin(); it != m_in_flight_jobs.end();) {
		if (it->m_data.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
			it++;
			continue;
		}

		// If the chunk is out of range now, don't mesh it
		if (world.GetChunk(it->m_pos))
		{
			if (uploads >= MAX_UPLOADS_PER_FRAME) {
				break;
			}

			std::unique_ptr<Mesh>& mesh = m_chunk_meshes[it->m_pos];
			if (!mesh) mesh = AcquireMesh();
			mesh->Upload(it->m_data.get());
			uploads++;
		}

		// Remove the mesh data from the hashset
		m_in_flight_positions.erase(it->m_pos);

		// In case it was re-dirtied while in flight
		try_queue(it->m_pos);

		// Erase it from the vector
		it = m_in_flight_jobs.erase(it);
	}
	profiler.Lap(Phase::Upload);

	profiler.End(m_dirty.size(), m_ready.size(), m_in_flight_jobs.size(), m_stats);
}

void WorldRenderer::RenderWorld(
	const glm::mat4& model, 
	const glm::mat4& view, 
	const glm::mat4& proj) 
{
	m_block_atlas.Bind(0);
	m_shader.Bind();
	m_shader.SetUniform("model", model);
	m_shader.SetUniform("view", view);
	m_shader.SetUniform("proj", proj);

	Frustum frustum(proj, view);
	for (auto& [chunk_pos, mesh] : m_chunk_meshes) {
		glm::vec3 min = chunk_pos * Chunk::SIZE;
		glm::vec3 max = min + float(Chunk::SIZE);
		if (!frustum.IntersectsBox(min, max)) continue;

		m_shader.SetUniform(m_chunk_uniform_loc, chunk_pos * Chunk::SIZE);
		mesh->Draw();
	}
}

void WorldRenderer::MeshChunk(
	const World& world, 
	const glm::ivec3& chunk_pos, 
	ThreadPool& thread_pool) 
{
	// Ensure the chunk data has been loaded
	if (!world.GetChunk(chunk_pos)) return;

	PaddedChunk padded_chunk;

	// Each chunk has 26 neighbours + the original chunk in the center = 27 chunks
	// Like a Rubix cube
	std::array<const Chunk*, 27> neighbours{};
	auto index = [](const glm::ivec3& offset) {
		return (offset.x + 1) + 3 * ((offset.y + 1) + 3 * (offset.z + 1));
	};

	// Populate the neighbours array
	for (int z = -1; z <= 1; z++) {
		for (int y = -1; y <= 1; y++) {
			for (int x = -1; x <= 1; x++) {
				glm::ivec3 neighbour(x, y, z);
				neighbours[index(neighbour)] = world.GetChunk(chunk_pos + neighbour);
			}
		}
	}

	const Chunk* center = neighbours[index({0, 0, 0})];

	auto is_full = [&](const glm::ivec3& offset) {
		const Chunk* chunk = neighbours[index(offset)];
		return chunk ? chunk->IsFull() : offset.y <= 0;
	};

	bool buried = center->IsFull();
	for (const FaceDef& face : FACES) {
		buried = buried && is_full(face.m_normal);
	}

	if (center->IsEmpty() || buried) {
		ReleaseMesh(chunk_pos);
		return;
	}

	// Fill in the blocks for the padded chunk
	// Trick is, given that pos runs from -1 to 16:
	// pos		pos >> 4		pos & 15
	// ---------------------------------
	// -1		-1				15
	// 0..15	0				0..15
	// 16		1				0
	// So pos >> 4 gives the chunk and pos & 15 gives the block inside the chunk
	for (int z = -1; z < PaddedChunk::SIZE - 1; z++) {
		for (int y = -1; y < PaddedChunk::SIZE - 1; y++) {
			for (int x = -1; x < PaddedChunk::SIZE - 1; x++) {
				glm::ivec3 pos(x, y, z);
				glm::ivec3 off = pos >> 4;
				const Chunk* src = neighbours[index(off)];
				padded_chunk.SetBlock(pos, src ? src->GetBlock(pos & 15) 
					: (off.y > 0 ? BlockId::Air : BlockId::Stone)); 
			}
		}
	}

	// Build the chunk mesh, and store the mesh data once complete (multithreaded)
	m_in_flight_jobs.emplace_back(
		chunk_pos,
		thread_pool.Submit([padded_chunk] {
			return BuildChunkMesh(padded_chunk);
	}));
	m_in_flight_positions.insert(chunk_pos);
}

void WorldRenderer::OnBlockChanged(const World& world, const glm::ivec3& world_pos) {
	glm::ivec3 chunk_pos = world_pos >> 4; // Equivalent to floor of divide by 16
	glm::ivec3 local_pos = world_pos & 15; // Block position inside the chunk

	auto mark = [&](const glm::ivec3& pos) {
		if (!world.GetChunk(pos)) return;
		m_dirty.insert(pos);

		// A negative distance puts it ahead of every chunk queued by distance. If it is still in
		// flight with the old blocks, the upload loop queues it again once that job finishes
		if (NeighboursReady(world, pos)) {
			m_ready.push({ -1, pos });
		}
	};

	mark(chunk_pos);

	// A block on the edge of its chunk is also a face neighbour of a block in the next chunk
	for (int axis = 0; axis < 3; axis++) {
		glm::ivec3 offset(0);
		if (local_pos[axis] == 0) {
			offset[axis] = -1;
		} else if (local_pos[axis] == Chunk::SIZE - 1) {
			offset[axis] = 1;
		} else {
			continue;
		}
		mark(chunk_pos + offset);
	}
}

void WorldRenderer::ReleaseMesh(const glm::ivec3& chunk_pos) {
	auto it = m_chunk_meshes.find(chunk_pos);
	if (it == m_chunk_meshes.end()) return;

	// Its old buffer contents are left in place, Upload() replaces them when it is reused
	m_spare_meshes.push_back(std::move(it->second));
	m_chunk_meshes.erase(it);
}

std::unique_ptr<Mesh> WorldRenderer::AcquireMesh() {
	if (m_spare_meshes.empty()) return std::make_unique<Mesh>();

	std::unique_ptr<Mesh> mesh = std::move(m_spare_meshes.back());
	m_spare_meshes.pop_back();
	return mesh;
}

bool WorldRenderer::NeighboursReady(const World& world, const glm::ivec3& chunk_pos) const {
	glm::ivec3 center = world.GetCenter();
	int render_dist = world.GetRenderDist();

	glm::ivec3 sides[] = {{ 1, 0, 0 }, { -1, 0, 0 }, { 0, 0, 1 }, { 0, 0, -1 }};
	for (const glm::ivec3& side : sides) {
		glm::ivec3 n = chunk_pos + side;
		int dist = std::max(std::abs(n.x - center.x), std::abs(n.z - center.z));

		// Within R and the it is yet to be loaded means that it is still in World's queue
		if (dist <= render_dist && !world.GetChunk(n)) {
			return false;
		}
	}
	
	return true;
}

}