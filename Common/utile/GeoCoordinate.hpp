#pragma once
#ifndef COMMON_UTILE_GEOCOORDINATE_HPP
#define COMMON_UTILE_GEOCOORDINATE_HPP

#include <string>
#include <optional>
#include <type_traits>

#include "DataTypes.hpp"
constexpr auto PI = 3.14159265358979323846;

namespace common
{
	namespace utile
	{
		constexpr auto DECIMALCOORDINATE_INVALID_VALUE = 257.0;
		typedef double DecimalCoordinate;
		typedef double Bearing;
		struct DegreesMinutesSeconds;

		DegreesMinutesSeconds DecimalToMinutesAndSeconds(const DecimalCoordinate& decimalDegrees);
		DecimalCoordinate DegreesMinutesSecondsToDecimalCoordinate(const DegreesMinutesSeconds& unit);
		std::optional<DecimalCoordinate> StringToDecimalCoordinates(std::string& value);
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

			GeoCoordinate() = default;
			GeoCoordinate(const GeoCoordinate<T>& obj)
			{
				this->latitude = obj.latitude;
				this->longitude = obj.longitude;
			}

			bool operator==(const GeoCoordinate& obj) const
			{
				return ((this->latitude == obj.latitude) && (this->longitude == obj.longitude));
			}

			// https://www.igismap.com/formula-to-find-bearing-or-heading-angle-between-two-points-latitude-longitude/
			// https://analyse-gps.com/gps-equations/bearing-of-2-gps-coordinates/
			std::optional<Bearing> calculateBearingToCoordinate(const GeoCoordinate<T>& coordinate) const
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

			LANE calculateDirection(const GeoCoordinate<T> coordinate)
			{
				DecimalCoordinate startlatitude = latitude;
				DecimalCoordinate startlongitude = longitude;
				DecimalCoordinate headinglatitude = coordinate.latitude;
				DecimalCoordinate headinglongitude = coordinate.longitude;

				return LANE::E;
			}
		};

		// FOR SOME KIND OF REASON CAN NOT ADD IT INSIDE CLASS
		template<typename T>
		std::optional<LANE> calculateDirection(const GeoCoordinate<T>& startcoordinate, const GeoCoordinate<T>& coordinate)
		{
			if (startcoordinate == coordinate)
			{
				return {};
			}

			DecimalCoordinate startlatitude = startcoordinate.latitude;
			DecimalCoordinate startlongitude = startcoordinate.longitude;
			DecimalCoordinate headinglatitude = coordinate.latitude;
			DecimalCoordinate headinglongitude = coordinate.longitude;
			
			DecimalCoordinate horizontalTraveledDistance = abs(headinglongitude - startlongitude);
			LANE headingHorizontal = (startlongitude < headinglongitude) ? LANE::E : LANE::W;
			DecimalCoordinate verticalTraveledDistance = abs(headinglatitude - startlatitude);
			LANE headingVertical = (startlatitude < headinglatitude) ? LANE::N : LANE::S;

			return ((horizontalTraveledDistance > verticalTraveledDistance) ? headingHorizontal : headingVertical);
		}
	} // namespace utile
} // namespace common

#endif // #COMMON_UTILE_GEOCOORDINATE_HPP