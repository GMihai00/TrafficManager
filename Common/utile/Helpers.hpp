#pragma once
#ifndef COMMON_UTILE_HELPERS
#define COMMON_UTILE_HELPERS

#include "DataTypes.hpp"
#include <optional>

namespace common
{
	namespace utile
	{
		std::optional<LANE> CharToLane(const char& lane)
		{
			if (lane == 'E') { return LANE::E; }
			if (lane == 'W') { return LANE::W; }
			if (lane == 'S') { return LANE::S; }
			if (lane == 'N') { return LANE::N; }
			return {};
		}

		std::optional<char> LaneToChar(const LANE& lane)
		{
			if (lane == LANE::E) { return 'E'; }
			if (lane == LANE::W) { return 'W'; }
			if (lane == LANE::S) { return 'S'; }
			if (lane == LANE::N) { return 'N'; }
			return {};
		}
	} // utile
} // common
#endif // #COMMON_UTILE_HELPERS