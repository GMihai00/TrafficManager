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
		std::atomic_bool shouldPause_ = true;
		std::atomic_bool shuttingDown_ = false;

		void process();
		int hexStringToInt(std::string& value);
		int calculateCheckSum(std::string NMEAString);
		std::string getNextValue(std::string& NMEAString, size_t& start);
		std::optional<GeoCoordinate<DecimalCoordinate>> parseGPGLLString(std::string NMEAString);
		std::optional<GeoCoordinate<DecimalCoordinate>> parseGPGGAString(std::string NMEAString);
		std::optional<GeoCoordinate<DecimalCoordinate>> parseGPRMCString(std::string NMEAString);
		std::optional<GeoCoordinate<DecimalCoordinate>> parseNMEAString(std::string& NMEAString);
	public:
		GPSAdapter() = delete;
		GPSAdapter(std::istream& inputStream);
		~GPSAdapter() noexcept;
		std::optional<GeoCoordinate<DecimalCoordinate>> getCurrentCoordinates();

		bool start();
		void pause();
		void stop();
	};

}
#endif // #MODEL_GPSADAPTER_HPP