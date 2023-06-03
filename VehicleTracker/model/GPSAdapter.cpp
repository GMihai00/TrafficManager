#include "GPSAdapter.hpp"

#include <functional>
#include <sstream>
#include <boost/algorithm/string.hpp>

#define CHECK_IF_STILL_VALID_POSITION(start) if (start == std::string::npos) { return std::nullopt; }

namespace model
{
	void GPSAdapter::process()
	{
		while (!shuttingDown_)
		{
			std::unique_lock<std::mutex> ulock(mutexProcess_);
			if ((lastCoordinates_.has_value() || shouldPause_) && !shuttingDown_)
				condVarProcess_.wait(ulock, [&] { return ((!lastCoordinates_.has_value() && !shouldPause_) || shuttingDown_); });

			if (shuttingDown_)
			{
				continue;
			}

			std::string NMEAString;
			inputStream_.sync();
			while (lastCoordinates_.has_value() == false)
			{
				NMEAString.clear();
				if (!std::getline(inputStream_, NMEAString))
					continue;

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
		stop();
		if (threadProcess_.joinable())
		{
			threadProcess_.join();
		}
	}

	bool GPSAdapter::start()
	{
		shouldPause_ = false;
		return true;
	}

	void GPSAdapter::pause()
	{
		shouldPause_ = true;
	}

	void GPSAdapter::stop()
	{
		shuttingDown_ = true;
		condVarProcess_.notify_one();
	}

	std::optional<GeoCoordinate<DecimalCoordinate>> GPSAdapter::getCurrentCoordinates()
	{
		std::unique_lock<std::mutex> ulock(mutexProcess_);
		if (!lastCoordinates_.has_value() && !shuttingDown_)
		{
			condVarProcess_.notify_one();
			condVarProcess_.wait(ulock, [&] { return lastCoordinates_.has_value() || shuttingDown_; });
		}

		if (shuttingDown_)
		{
			return std::nullopt;
		}

		common::utile::GeoCoordinate currentCoordinate = lastCoordinates_.value();
		lastCoordinates_ = std::nullopt;
		return currentCoordinate;
	}

	std::string GPSAdapter::getNextValue(std::string& NMEAString, size_t& start)
	{
		std::string nexValue = "";
		std::size_t end = NMEAString.find_first_of(",", start);
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
		std::optional<GeoCoordinate<DecimalCoordinate>> rez = std::nullopt;
		rez = parseGPGLLString(NMEAString);
		if (rez.has_value())
		{
			return rez;
		}

		rez = parseGPGGAString(NMEAString);
		if (rez.has_value())
		{
			return rez;
		}

		rez = parseGPRMCString(NMEAString);
		if (rez.has_value())
		{
			return rez;
		}

		return std::nullopt;
	}

	std::optional<GeoCoordinate<DecimalCoordinate>> GPSAdapter::parseGPGLLString(std::string NMEAString)
	{
		//"$GPGLL", <lat>, "N/S", <lon>, "E/W", <time>(hhmmss.sss), "A/V", "A/D/E/M/N" "*" <checksum> "CR/LF"(ignored in our case)
		GeoCoordinate<DecimalCoordinate> rez{};
		size_t start = 0;

		//$GPGLL
		std::string value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);
		if (value != "$GPGLL") { return std::nullopt; }

		// <lat>
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		auto latitude = common::utile::StringToDecimalCoordinates(value);
		if (!latitude.has_value()) { return std::nullopt; }

		// N/S
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);
		if (!(value == "N" || value == "S")) { return std::nullopt; }
		if (value == "S") { latitude = -latitude.value(); }

		// <lon>
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		auto longitude = common::utile::StringToDecimalCoordinates(value);
		if (!longitude.has_value()) { return std::nullopt; }

		//take E/W
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);
		if (!(value == "E" || value == "W")) { return std::nullopt; }
		if (value == "W") { longitude = -longitude.value(); }

		// <time>(hhmmss.sss) IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// A/V if V return {}
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);
		if (value == "V" ) { return std::nullopt; }

		// "A/D/E/M/N" useless so just skip it
		start++;
		CHECK_IF_STILL_VALID_POSITION(start);

		// "*" <checksum> if not matching return {}
		if (NMEAString[start] != '*') { return std::nullopt; }
		start++;
		CHECK_IF_STILL_VALID_POSITION(start);

		value = NMEAString.substr(start, NMEAString.size() - start + 1);
		if (calculateCheckSum(std::string(NMEAString.substr(1, start - 2))) != hexStringToInt(value)) { return std::nullopt; }

		rez.latitude = latitude.value();
		rez.longitude = longitude.value();
		return rez;
	}

	std::optional<GeoCoordinate<DecimalCoordinate>> GPSAdapter::parseGPGGAString(std::string NMEAString)
	{
		//"$GPGGA", <time>(hhmmss.sss), <lat>, "N/S", <lon>, "E/W", <Position Fix Indicator>, <Satellites used>, <HDOP>, <MSL Altitude>, <Units>, 
		//<Geoid Separation>, <Units>, <Age of diff. corr.>, <Diff. ref. station ID> "*" <checksum> "CR/LF"(ignored in our case)
		GeoCoordinate<DecimalCoordinate> rez{};
		size_t start = 0;

		//$GPGGA
		std::string value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);
		if (value != "$GPGGA") { return std::nullopt; }

		// <time>(hhmmss.sss) IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// <lat>
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		auto latitude = common::utile::StringToDecimalCoordinates(value);
		if (!latitude.has_value()) { return std::nullopt; }

		// N/S
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);
		if (!(value == "N" || value == "S")) { return std::nullopt; }
		if (value == "S") { latitude = -latitude.value(); }

		// <lon>
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		auto longitude = common::utile::StringToDecimalCoordinates(value);
		if (!longitude.has_value()) { return std::nullopt; }

		//take E/W
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);
		if (!(value == "E" || value == "W")) { return std::nullopt; }
		if (value == "W") { longitude = -longitude.value(); }


		// <Position Fix Indicator> IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// <Satellites used> IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// <HDOP> IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// <MSL Altitude> IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// <Units> IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// <Geoid Separation> IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);


		// <Units> IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);


		// <Age of diff. corr.> IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// <Diff. ref. station ID> IGNORED
		while (start < NMEAString.size() && NMEAString[start] != '*') { start++; }

		if (start >= NMEAString.size() || NMEAString[start] != '*') { return std::nullopt; }
		start++;
		CHECK_IF_STILL_VALID_POSITION(start);

		value = NMEAString.substr(start, NMEAString.size() - start + 1);
		if (calculateCheckSum(std::string(NMEAString.substr(1, start - 2))) != hexStringToInt(value)) { return std::nullopt; }

		rez.latitude = latitude.value();
		rez.longitude = longitude.value();
		return rez;
	}


	std::optional<GeoCoordinate<DecimalCoordinate>> GPSAdapter::parseGPRMCString(std::string NMEAString)
	{
		//"$GPRMC", <time>(hhmmss.sss), "A/V", <lat>, "N/S", <lon>, "E/W", <Speed over ground>, <Course over ground>, <Date>, <Magnetic Variation>, "A/D/E" "*" <checksum> "CR/LF"(ignored in our case)
		GeoCoordinate<DecimalCoordinate> rez{};
		size_t start = 0;

		//$GPRMC
		std::string value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);
		if (value != "$GPRMC") { return std::nullopt; }

		// <time>(hhmmss.sss) IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// A/V if V return {}
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);
		if (value == "V") { return std::nullopt; }

		// <lat>
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		auto latitude = common::utile::StringToDecimalCoordinates(value);
		if (!latitude.has_value()) { return std::nullopt; }

		// N/S
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);
		if (!(value == "N" || value == "S")) { return std::nullopt; }
		if (value == "S") { latitude = -latitude.value(); }

		// <lon>
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		auto longitude = common::utile::StringToDecimalCoordinates(value);
		if (!longitude.has_value()) { return std::nullopt; }

		//take E/W
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);
		if (!(value == "E" || value == "W")) { return std::nullopt; }
		if (value == "W") { longitude = -longitude.value(); }

		// <Speed over ground> IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// <Course over ground> IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// <Date> IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// <Magnetic Variation> IGNORED
		value = getNextValue(NMEAString, start);
		CHECK_IF_STILL_VALID_POSITION(start);

		// "A/D/E" useless so just skip it
		start++;
		CHECK_IF_STILL_VALID_POSITION(start);

		// "*" <checksum> if not matching return {}
		if (NMEAString[start] != '*') { return std::nullopt; }
		start++;
		CHECK_IF_STILL_VALID_POSITION(start);

		value = NMEAString.substr(start, NMEAString.size() - start + 1);
		if (calculateCheckSum(std::string(NMEAString.substr(1, start - 2))) != hexStringToInt(value)) { return std::nullopt; }

		rez.latitude = latitude.value();
		rez.longitude = longitude.value();
		return rez;
	}
}