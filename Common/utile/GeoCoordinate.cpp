#include "GeoCoordinate.hpp"
#include <vector>
#include <boost/algorithm/string.hpp>

namespace common
{
	namespace utile
	{

		bool DegreesMinutesSeconds::operator==(const DegreesMinutesSeconds& obj) const
		{
			return (this->degrees == obj.degrees) &&
				(this->minutues == obj.minutues) &&
				(this->seconds == obj.seconds) &&
				(this->direction == obj.direction);
		}

		DegreesMinutesSeconds DecimalToMinutesAndSeconds(const DecimalCoordinate& decimalDegrees)
		{
			DegreesMinutesSeconds rez = {};
			rez.degrees = floor(decimalDegrees);
			rez.minutues = ((floor(decimalDegrees) - rez.degrees) * 60);
			rez.seconds = ((floor(decimalDegrees) - rez.degrees - rez.minutues / 60) * 3600);
			return rez;
		}

		DecimalCoordinate DegreesMinutesSecondsToDecimalCoordinate(const DegreesMinutesSeconds& unit)
		{
			double rez = unit.degrees + unit.minutues / 60 + unit.seconds / 3600;
			return (unit.direction == 'N' || unit.direction == 'W') ? rez : -rez;
		}

		std::optional<DegreesMinutesSeconds> StringToDegreesMinutesAndSeconds(const std::string& value)
		{
			//EX: "45° 02' 60.0\" N"
			std::vector<std::string> symbloToFind = { "°", "'", "\"" };
			std::vector<double> numbersFound;
			// FIND numeric values
			std::size_t last = 0;
			std::size_t next;
			for (const auto& symbol : symbloToFind)
			{
				next = value.find(symbol);
				if (next == std::string::npos) { return {}; }
				try
				{
					auto substring = value.substr(last, next - last);
					boost::algorithm::trim(substring);
					numbersFound.push_back(std::stod(substring));
				}
				catch (...)
				{
					return {};
				}
				last = next + 1;
			}
			DegreesMinutesSeconds rez{};
			rez.degrees = numbersFound[0];
			rez.minutues = numbersFound[1];
			rez.seconds = numbersFound[2];
			// read last character for direction
			rez.direction = toupper(value[value.size() - 1]);
			if (!(rez.direction == 'N' || rez.direction == 'S' || rez.direction == 'E' || rez.direction == 'W'))
			{
				return {};
			}
			return rez;
		}

		std::optional<DecimalCoordinate> StringToDecimalCoordinates(const std::string& value)
		{
			try
			{
				boost::algorithm::trim(value);
				return std::stod(value);
			}
			catch (...)
			{
				return {};
			}
		}
	} // namespace utile
} // namespace common