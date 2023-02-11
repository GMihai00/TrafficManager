#pragma once
#ifndef COMMON_UTILE_GEOCOORDINATE_HPP
#define COMMON_UTILE_GEOCOORDINATE_HPP

#include <string>
#include <optional>

namespace common
{
	namespace utile
	{
		struct PositionalUnit
		{
			double degrees;
			double minutues;
			double seconds;
			char direction;
		};

		struct GeoCoordinate
		{
			PositionalUnit latitude;
			PositionalUnit longitude;
		};

		std::optional<PositionalUnit> decimalToMinutesAndSeconds(long double decimalDegrees)
		{
			PositionalUnit rez = {};
			// TO DO

			return rez;
		}
	}
} // namespace common

#endif // #COMMON_UTILE_GEOCOORDINATE_HPP