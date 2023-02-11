#pragma once
#ifndef COMMON_UTILE_GEOCOORDINATE_HPP
#define COMMON_UTILE_GEOCOORDINATE_HPP

#include <string>
#include <optional>
#include <type_traits>

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
			std::optional<Bearing> calculateBearingToCoordinate(GeoCoordinate<T> coordinate)
			{
				try
				{
					Bearing bearing;
					DecimalCoordinate decimalLatitude = latitude;
					DecimalCoordinate decimalLongitude = longitude;
					
					// TO CONTINUE BASED ON WEBSITE
					double X = cos() * sin();
					double Y = cos() * sin() - sin() * cos() * cos();
					
					return atan2(X, Y); // radians to degrees
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