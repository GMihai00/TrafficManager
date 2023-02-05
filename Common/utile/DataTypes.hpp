#pragma once
#ifndef COMMON_UTILE_DATATYPES_HPP
#define COMMON_UTILE_DATATYPES_HPP

#include <iostream>
#include <memory>

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
	}
}
#endif // #UTILE_DATATYPES_HPP
