#include "WorldRenderer.hpp"
#include "ChunkMesher.hpp"
#include "graphics/Frustum.hpp"

#include <unordered_set>
#include <chrono>

namespace mcc {

using Clock = std::chrono::steady_clock;
using ms = std::chrono::duration<double, std::milli>;

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
	// Erase the unloaded chunks from the meshes
	for (const glm::ivec3& pos : changes.m_unloaded) {
		m_chunk_meshes.erase(pos);
		m_dirty.erase(pos);
	}

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

	// Collect dirty chunks whose neighbours are ready
	glm::ivec3 center = world.GetCenter();
	auto dist_squared = [&](const glm::ivec3& p) {
		int dx = p.x - center.x;
		int dz = p.z - center.z;
		return dx * dx + dz * dz;
	};

	// Sort the dirty chunks to be nearest first in the queue
	std::vector<glm::ivec3> ready;
	for (const glm::ivec3& pos : m_dirty) {
		if (!m_in_flight_positions.contains(pos) && NeighboursReady(world, pos)) {
			ready.push_back(pos);
		}
	}
	std::sort(ready.begin(), ready.end(), [&](const glm::ivec3& a, const glm::ivec3& b) {
			return dist_squared(a) < dist_squared(b);
		}
	);

	std::size_t max_in_flights = thread_pool.Size() * 2;
	auto start = Clock::now();
	for (const auto& pos : ready) {
		if (ms(Clock::now() - start).count() > MESH_FRAMETIME_MS) break;
		if (m_in_flight_positions.size() >= max_in_flights) break;

		MeshChunk(world, pos, thread_pool);
		m_dirty.erase(pos);
	}

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
			if (!mesh) mesh = std::make_unique<Mesh>();
			mesh->Upload(it->m_data.get());
			uploads++;
		}

		// Remove the mesh data from the hashset
		m_in_flight_positions.erase(it->m_pos);

		// Erase it from the vector
		it = m_in_flight_jobs.erase(it);
	}
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
		m_chunk_meshes.erase(chunk_pos);
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