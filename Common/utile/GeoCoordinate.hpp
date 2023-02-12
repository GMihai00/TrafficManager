#pragma once
#ifndef COMMON_UTILE_GEOCOORDINATE_HPP
#define COMMON_UTILE_GEOCOORDINATE_HPP

#include <string>
#include <optional>
#include <type_traits>

constexpr auto PI = 3.14159265358979323846;

namespace common
{
	namespace utile
	{
		typedef double DecimalCoordinate;
		typedef double Bearing;
		struct DegreesMinutesSeconds;

		DegreesMinutesSeconds DecimalToMinutesAndSeconds(const DecimalCoordinate& decimalDegrees);
		DecimalCoordinate DegreesMinutesSecondsToDecimalCoordinate(const DegreesMinutesSeconds& unit);
		std::optional<DecimalCoordinate> StringToDecimalCoordinates(const std::string& value);
		std::optional<DegreesMinutesSeconds> StringToDegreesMinutesAndSeconds(const std::string& value);

		struct DegreesMinutesSeconds
		{
			double degrees;
			double minutues;
			double seconds;
			char direction;

			bool operator==(const DegreesMinutesSeconds& obj) const;
			operator DecimalCoordinate() const
			{
				return DegreesMinutesSecondsToDecimalCoordinate(*this);
			}
		};

		template<typename T>
		struct GeoCoordinate
		{
			T latitude;
			T longitude;

			bool operator==(const GeoCoordinate& obj) const
			{
				return ((this->latitude == obj.latitude) && (this->longitude == obj.longitude));
			}

			// https://www.igismap.com/formula-to-find-bearing-or-heading-angle-between-two-points-latitude-longitude/
			// https://analyse-gps.com/gps-equations/bearing-of-2-gps-coordinates/
			std::optional<Bearing> calculateBearingToCoordinate(GeoCoordinate<T> coordinate)
			{
				try
				{
					DecimalCoordinate latFirst = latitude;
					DecimalCoordinate latSecond = coordinate.latitude;
					DecimalCoordinate delta = coordinate.longitude - longitude;
					double X = cos(latSecond) * sin(delta);
					double Y = cos(latFirst) * sin(latSecond) - sin(latFirst) * cos(latSecond) * cos(delta);
					
					return atan2(X, Y) * (180.0 / PI);
				}
				catch (...)
				{
					return {};
				}
			}
		};
	} // namespace utile
} // namespace common

#endif // #COMMON_UTILE_GEOCOORDINATE_HPP