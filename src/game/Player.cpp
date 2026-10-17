#include "game/Player.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace mcc {

namespace {

// All distances are in blocks and all times in seconds. The values are close to Minecraft's
constexpr double WALK_SPEED = 4.3;
constexpr double SPRINT_SPEED = 5.6;
constexpr double GRAVITY = 32.0;
constexpr double TERMINAL_SPEED = 78.0;  // Fastest fall speed
constexpr double JUMP_SPEED = 9.0;       // v^2 / 2g = 1.27 blocks high: clears 1 block, not 2

// How quickly walking speed changes (blocks per second, per second). Low in the air, so a jump
// keeps its momentum and steering mid-air is sluggish, as in real life
constexpr double GROUND_ACCEL = 40.0;
constexpr double AIR_ACCEL = 8.0;

constexpr double FLY_SPEED = 20.0;
constexpr double FLY_SPRINT_SPEED = 100.0;

// Frames longer than this are cut short, so a stall can't fling the player through the world
constexpr double MAX_FRAME_DT = 0.1;

// Physics runs in steps of at most this long, so it behaves the same at any framerate
constexpr double MAX_STEP_DT = 1.0 / 120.0;

// Floating point slack when checking whether two boxes touch. Without it, being 0.0000000001
// inside the floor (from rounding) counts as overlapping it, and the floor blocks walking
constexpr double EPS = 1e-7;

// Whether the block at block_pos stops the player
bool BlocksMovement(const World& world, const glm::ivec3& block_pos) {
	glm::ivec3 chunk_pos = block_pos >> 4;

	// Solid floor under the world, so the player can't fall out of it
	if (chunk_pos.y < MIN_CHUNK_Y) return true;

	// Open sky above the world
	if (chunk_pos.y >= MAX_CHUNK_Y) return false;

	// Not loaded yet: treated as a wall, so the player can't fall or walk into terrain that isn't
	// there yet (e.g. at spawn, before the ground has loaded)
	const Chunk* chunk = world.GetChunk(chunk_pos);
	if (!chunk) return true;

	return IsSolid(chunk->GetBlock(block_pos & 15));
}

// Calls fn with the box of every block inside region that stops the player
template <typename Fn>
void ForEachBlockingBox(const World& world, const AABB& region, Fn&& fn) {
	glm::ivec3 min(glm::floor(region.m_min));
	glm::ivec3 max(glm::floor(region.m_max));

	for (int x = min.x; x <= max.x; x++) {
		for (int y = min.y; y <= max.y; y++) {
			for (int z = min.z; z <= max.z; z++) {
				glm::ivec3 pos(x, y, z);
				if (BlocksMovement(world, pos)) {
					fn(AABB::Block(pos));
				}
			}
		}
	}
}

// How far box can move along axis (0 = x, 1 = y, 2 = z), up to d, before it hits block.
// A block is only in the way if it lines up with the box on the two other axes and sits on the
// side the box is moving towards. A block the box is already inside is ignored, so the player can
// always move out of it
double ClipAxis(const AABB& box, const AABB& block, int axis, double d) {
	for (int other = 0; other < 3; other++) {
		if (other == axis) continue;
		bool apart =
			box.m_max[other] <= block.m_min[other] + EPS ||
			box.m_min[other] >= block.m_max[other] - EPS;
		if (apart) return d;
	}

	if (d > 0.0 && box.m_max[axis] <= block.m_min[axis] + EPS) {
		d = std::min(d, block.m_min[axis] - box.m_max[axis]);
	} else if (d < 0.0 && box.m_min[axis] >= block.m_max[axis] - EPS) {
		d = std::max(d, block.m_max[axis] - box.m_min[axis]);
	}
	return d;
}

}

Player::Player(const glm::dvec3& feet_pos)
	: m_pos(feet_pos) {}

void Player::Update(const World& world, const PlayerInput& input, float dt) {
	double frame_dt = std::min(static_cast<double>(dt), MAX_FRAME_DT);

	// Flying ignores physics and collisions entirely
	if (m_flying) {
		glm::dvec3 dir(input.m_move);
		if (input.m_jump) dir.y += 1.0;
		if (input.m_descend) dir.y -= 1.0;

		double speed = input.m_sprint ? FLY_SPRINT_SPEED : FLY_SPEED;
		m_pos += dir * speed * frame_dt;
		return;
	}

	int steps = std::max(1, static_cast<int>(std::ceil(frame_dt / MAX_STEP_DT)));
	double step_dt = frame_dt / steps;
	for (int i = 0; i < steps; i++) {
		Step(world, input, step_dt);
	}
}

void Player::ToggleFlying() {
	m_flying = !m_flying;
	m_vel = glm::dvec3(0.0);
	m_on_ground = false;
}

AABB Player::Box() const {
	glm::dvec3 half(WIDTH * 0.5, 0.0, WIDTH * 0.5);
	return { m_pos - half, m_pos + half + glm::dvec3(0.0, HEIGHT, 0.0) };
}

glm::vec3 Player::EyePos() const {
	return glm::vec3(m_pos + glm::dvec3(0.0, EYE_HEIGHT, 0.0));
}

void Player::Step(const World& world, const PlayerInput& input, double dt) {
	// Walking: change the horizontal velocity towards the one the player wants, by at most
	// accel * dt. Starting, stopping and turning all take a moment, instead of being instant
	glm::dvec2 target = glm::dvec2(input.m_move.x, input.m_move.z)
		* (input.m_sprint ? SPRINT_SPEED : WALK_SPEED);
	glm::dvec2 vel(m_vel.x, m_vel.z);
	glm::dvec2 change = target - vel;

	double max_change = (m_on_ground ? GROUND_ACCEL : AIR_ACCEL) * dt;
	double change_len = glm::length(change);
	if (change_len > max_change) {
		change *= max_change / change_len;
	}

	vel += change;
	m_vel.x = vel.x;
	m_vel.z = vel.y;

	// Jumping. Only from the ground, and holding the key jumps again on landing, like Minecraft
	if (input.m_jump && m_on_ground) {
		m_vel.y = JUMP_SPEED;
	}

	// Gravity
	m_vel.y = std::max(m_vel.y - GRAVITY * dt, -TERMINAL_SPEED);

	MoveAndCollide(world, m_vel * dt);
}

void Player::MoveAndCollide(const World& world, const glm::dvec3& motion) {
	m_on_ground = false;

	// Each axis is moved and collided separately, which is what lets the player slide along a
	// wall instead of sticking to it. Y goes first, like in Minecraft: landing on the ground
	// before moving sideways
	constexpr std::array<int, 3> AXES = { 1, 0, 2 };
	for (int axis : AXES) {
		double d = motion[axis];
		if (d == 0.0) continue;

		// Every block the box could touch while moving: its current box stretched by d
		AABB box = Box();
		AABB swept = box;
		if (d > 0.0) {
			swept.m_max[axis] += d;
		} else {
			swept.m_min[axis] += d;
		}

		// Shorten the move to stop at the nearest block in the way
		double allowed = d;
		ForEachBlockingBox(world, swept, [&](const AABB& block) {
			allowed = ClipAxis(box, block, axis, allowed);
		});

		m_pos[axis] += allowed;

		// Hit something: stop moving that way. Hitting something below means standing on it
		if (allowed != d) {
			m_vel[axis] = 0.0;
			if (axis == 1 && d < 0.0) m_on_ground = true;
		}
	}
}

}
