#pragma once
#ifndef COMMON_UTILE_GEOCOORDINATE_HPP
#define COMMON_UTILE_GEOCOORDINATE_HPP

#include <string>
#include <optional>
#include <type_traits>
#include "boost/algorithm/string.hpp"

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

			GeoCoordinate(const std::string& input)
			{
				std::vector<std::string> val;
				boost::split(val, input, boost::is_any_of(","));
				if (val.size() != 2)
					throw std::runtime_error("Invalid input");
				latitude = std::stod(val[0]);
				longitude = std::stod(val[1]);
			}
			GeoCoordinate() = default;
			GeoCoordinate(const GeoCoordinate<T>& obj)
			{
				this->latitude = obj.latitude;
				this->longitude = obj.longitude;
			}

			bool operator==(const GeoCoordinate& obj) const
			{
				return ((std::fabs(this->latitude - obj.latitude) < 0.00001) && (std::fabs(this->longitude - obj.longitude) < 0.00001));
			}

			// SHOULD SWITCH TO BEARING IN THE END FOR NOW JUST USING S,E,N,W
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
		};

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