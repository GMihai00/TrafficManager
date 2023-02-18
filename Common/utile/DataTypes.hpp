#pragma once
#ifndef COMMON_UTILE_DATATYPES_HPP
#define COMMON_UTILE_DATATYPES_HPP

#include <iostream>
#include <memory>
#include <optional>

namespace common
{
	namespace utile
	{
		enum class LANE : uint8_t
		{
			E = 0,
			W,
			N,
			S
		};

		std::optional<LANE> CharToLane(const char& lane);
		std::optional<char> LaneToChar(const LANE& lane);
	} // namespace utile
} // namespace common
#endif // #COMMON_UTILE_DATATYPES_HPP
