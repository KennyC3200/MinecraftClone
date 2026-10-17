#pragma once

#include "game/AABB.hpp"
#include "world/World.hpp"

#include <glm/glm.hpp>

namespace mcc {

// What the player wants to do this frame, already turned from keys into directions by the Engine
struct PlayerInput {
	glm::vec3 m_move{ 0.0f }; // Horizontal direction to walk in, length 1 (or 0 to stand still)
	bool m_jump = false;      // Walking: jump. Flying: go up
	bool m_descend = false;   // Flying: go down
	bool m_sprint = false;
};

// The player's body: a 0.6 x 1.8 x 0.6 box that falls, jumps and collides with solid blocks.
// It can also fly, which turns physics off and lets it move freely through blocks
class Player final {
public:
	// feet_pos is the centre of the bottom of the box
	explicit Player(const glm::dvec3& feet_pos);

	void Update(const World& world, const PlayerInput& input, float dt);

	void ToggleFlying();
	bool IsFlying() const { return m_flying; }

	AABB Box() const;
	glm::vec3 EyePos() const;

	static constexpr double WIDTH = 0.6;
	static constexpr double HEIGHT = 1.8;
	static constexpr double EYE_HEIGHT = 1.62;

private:
	// One physics step. Update splits long frames into several of these
	void Step(const World& world, const PlayerInput& input, double dt);

	// Moves by motion, stopping at solid blocks. Sets m_on_ground and zeroes the velocity on any
	// axis that hit something
	void MoveAndCollide(const World& world, const glm::dvec3& motion);

	glm::dvec3 m_pos;            // Centre of the bottom of the box
	glm::dvec3 m_vel{ 0.0 };     // Blocks per second
	bool m_on_ground = false;
	bool m_flying = false;
};

}
