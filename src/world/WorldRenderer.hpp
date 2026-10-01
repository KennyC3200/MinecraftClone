#pragma once

#include "graphics/Shader.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Texture.hpp"
#include "world/World.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <memory>

namespace mcc {

class WorldRenderer final {
public:
	WorldRenderer();

	Mesh* GetChunkMesh(const glm::ivec3& chunk_pos);
	const Mesh* GetChunkMesh(const glm::ivec3& chunk_pos) const;

	void RenderWorld(const glm::mat4& model, const glm::mat4& view, const glm::mat4 proj);
	void MeshChunk(const World& world, const glm::ivec3& chunk_pos);

private:
	std::unordered_map<glm::ivec3, std::unique_ptr<Mesh>> m_chunk_meshes;
	Shader m_shader;
	Texture m_block_atlas;
};

}