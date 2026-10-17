#pragma once

#include "world/Block.hpp"

#include <array>
#include <cstddef>

namespace mcc {

class Hotbar final {
public:
	static constexpr std::size_t SIZE = 9;

	void Select(std::size_t slot) {
		if (slot < SIZE) m_selected = slot;
	}

	std::size_t Selected() const { return m_selected; }
	BlockId Slot(std::size_t slot) const { return m_slots.at(slot); }
	BlockId SelectedBlock() const { return m_slots[m_selected]; }

private:
	std::array<BlockId, SIZE> m_slots = {
		BlockId::Grass, BlockId::Dirt, BlockId::Stone,
		BlockId::Air,   BlockId::Air,  BlockId::Air,
		BlockId::Air,   BlockId::Air,  BlockId::Air,
	};
	std::size_t m_selected = 0;
};

}
