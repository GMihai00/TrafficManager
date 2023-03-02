#include "GPSAdapter.hpp"

#include <functional>
#include <sstream>
#include <boost/algorithm/string.hpp>

namespace model
{
	void GPSAdapter::process()
	{
		while (true)
		{
			std::unique_lock<std::mutex> ulock(mutexProcess_);
			condVarProcess_.wait(ulock, [&] { return (lastCoordinates_.has_value() == false); });

			std::string NMEAString;
			inputStream_.sync();
			while (lastCoordinates_.has_value() == false)
			{
				NMEAString.clear();
				std::getline(inputStream_, NMEAString);

				auto maybeCoordinate = parseNMEAString(NMEAString);
				if (maybeCoordinate.has_value())
				{
					lastCoordinates_ = maybeCoordinate;
				}
			}
			condVarProcess_.notify_one();
		}

	}

	GPSAdapter::GPSAdapter(std::istream& inputStream) :
		inputStream_(inputStream)
	{
		threadProcess_ = std::thread(std::bind(&GPSAdapter::process, this));
	}

	GPSAdapter::~GPSAdapter()
	{
		if (threadProcess_.joinable())
		{
			threadProcess_.join();
		}
	}

	GeoCoordinate<DecimalCoordinate> GPSAdapter::getCurrentCoordinates()
	{
		condVarProcess_.notify_one();
		std::unique_lock<std::mutex> ulock(mutexProcess_);
		condVarProcess_.wait(ulock, [&] { return lastCoordinates_.has_value(); });

		common::utile::GeoCoordinate currentCoordinate = lastCoordinates_.value();
		lastCoordinates_ = {};
		return currentCoordinate;
	}

	std::string GPSAdapter::getNextValue(std::string& NMEAString, size_t& start)
	{
		std::string nexValue = "";
		std::size_t end = NMEAString.find_first_of(",");
		if (end != std::string::npos)
		{
			nexValue = NMEAString.substr(start, end - start);
			boost::algorithm::trim(nexValue);
		}
		start = end + 1;
		return nexValue;
	}

	int GPSAdapter::calculateCheckSum(std::string NMEAString)
	{
		int checksum = 0;
		for (auto character : NMEAString)
		{
			checksum ^= (int)character;
		}
		return checksum;
	}
	int GPSAdapter::hexStringToInt(std::string& value)
	{
		int rez;
		std::istringstream(value) >> std::hex >> rez;
		return rez;
	}

	std::optional<GeoCoordinate<DecimalCoordinate>> GPSAdapter::parseNMEAString(std::string& NMEAString)
	{
		//"$GPGLL", <lat>, "N/S", <lon>, "E/W", <utc>, "A/V", "A/D/E/M/N" "*" <checksum> "CR/LF"(ignored in our case)
		GeoCoordinate<DecimalCoordinate> rez{};
		size_t start = 0;
        #define CHECK_IF_STILL_VALID_POSITION if (start == std::string::npos) { return {}; }

		//$GPGLL
		std::string value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION;
		if (value != "$GPGLL") { return {}; }

		// <lat>
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION;

		auto latitude = common::utile::StringToDecimalCoordinates(value);
		if (!latitude.has_value()) { return {}; }

		// N/S
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION;
		if (!(value == "N" || value == "S")) { return {}; }
		if (value == "S") { latitude = -latitude.value(); }

		// <lon>
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION;

		auto longitude = common::utile::StringToDecimalCoordinates(value);
		if (!longitude.has_value()) { return {}; }

		//take E/W
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION;
		if (!(value == "E" || value == "W")) { return {}; }
		if (value == "W") { longitude = -longitude.value(); }

		// utc IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION;

		// A/V if V return {}
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION;
		if (value == "V" ) { return {}; }

		// "A/D/E/M/N" useless
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION;

		// "*" <checksum> if not matching return {}
		if (NMEAString[start] != '*') { return {}; }
		value = NMEAString.substr(++start, NMEAString.size() - start + 1);
		if (calculateCheckSum(std::string(NMEAString.substr(0, start - 1))) != hexStringToInt(value)) { return {}; }

		rez.latitude = latitude.value();
		rez.longitude = longitude.value();
		return rez;
	}
}