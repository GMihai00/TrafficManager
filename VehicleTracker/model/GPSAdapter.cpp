#include "GPSAdapter.hpp"

#include <functional>

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
		inputStream_(inputStream_)
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

	bool GPSAdapter::start()
	{

	}

	void GPSAdapter::stop()
	{

	}

	common::utile::GeoCoordinate GPSAdapter::getCurrentCoordinates()
	{
		condVarProcess_.notify_one();
		std::unique_lock<std::mutex> ulock(mutexProcess_);
		condVarProcess_.wait(ulock, [&] { return lastCoordinates_.has_value(); });

		common::utile::GeoCoordinate currentCoordinate = lastCoordinates_.value();
		lastCoordinates_ = {};
		return currentCoordinate;
	}

	std::string GPSAdapter::getNextValue(std::string & NMEAString, size_t& start)
	{
		std::string nexValue = "";
		std::size_t end = NMEAString.find_first_of(",");
		if (end != std::string::npos)
		{

		}
		start = end;
		return nexValue;
	}

	// TO DO
	std::optional<common::utile::GeoCoordinate> GPSAdapter::parseNMEAString(std::string& NMEAString)
	{
		//"$GPGLL", <lat>, "N/S", <lon>, "E/W", <utc>, "A/V", "A/D/E/M/N" "*" <checksum> "CR/LF"(ignored in our case)
		common::utile::GeoCoordinate rez{};
		size_t start = 0;
		auto value =  getNextValue(NMEAString, start);
		if (start == std::string::npos || value != "$GPGLL")
		{
			return {};
		}


		long double decimalLatitude = std::stold();
		auto latitude = common::utile::decimalToMinutesAndSeconds(decimalLatitude);
		if (!latitude.has_value())
			return {};

		//take N/S
		rez.latitude = latitude.value();

		long double decimalLongitude = std::stold();
		auto longitude = common::utile::decimalToMinutesAndSeconds(decimalLongitude);
		if (!longitude.has_value())
			return {};

		//take E/W
		rez.longitude = longitude.value();

		// utc useless 

		// A/V if V return {}

		// "A/D/E/M/N" useless

		// "*" <checksum> if not matching return {}
		return rez;
	}
}