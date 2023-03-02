#pragma once
#ifndef MODEL_GPSADAPTER_HPP
#define MODEL_GPSADAPTER_HPP

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <optional>

#include "utile/GeoCoordinate.hpp"

namespace model
{
	using namespace common::utile;
	// TO DO: WRITE IN LICENCE ABOUT NMEA STRINGS
	// USED FOR PARSING NMEA GPGLL STRING// https://docs.novatel.com/OEM7/Content/Logs/GPGLL.htm
	class GPSAdapter
	{
	protected:
		std::optional<GeoCoordinate<DecimalCoordinate>> lastCoordinates_;
		std::istream& inputStream_;
		std::thread threadProcess_;
		std::mutex mutexProcess_;
		std::condition_variable condVarProcess_;

		void process();
		int hexStringToInt(std::string& value);
		int calculateCheckSum(std::string NMEAString);
		std::string getNextValue(std::string& NMEAString, size_t& start);
		std::optional<GeoCoordinate<DecimalCoordinate>> parseNMEAString(std::string& NMEAString);
	public:
		GPSAdapter() = delete;
		GPSAdapter(std::istream& inputStream);
		~GPSAdapter() noexcept;
		GeoCoordinate<DecimalCoordinate> getCurrentCoordinates();
	};

}
#endif // #MODEL_GPSADAPTER_HPP