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
	// TO DO: WRITE IN LICENCE ABOUT NMEA STRINGS
	// USED FOR PARSING NMEA GPGLL STRING// https://docs.novatel.com/OEM7/Content/Logs/GPGLL.htm
	// N,E POSITIVE VALUE; S,W NEGATIVE VALUE
	// degrees, minutes, seconds from  decimal degrees
	class GPSAdapter
	{
	protected:
		// if I don't need data I just flush the buffer
		std::optional<common::utile::GeoCoordinate> lastCoordinates_;
		std::istream& inputStream_;
		std::thread threadProcess_;
		std::mutex mutexProcess_;
		std::condition_variable condVarProcess_;

		void process();

		std::string getNextValue(std::string& NMEAString, size_t& start);
		std::optional<common::utile::GeoCoordinate> parseNMEAString(std::string& NMEAString);
	public:
		GPSAdapter() = delete;
		GPSAdapter(std::istream& inputStream);
		~GPSAdapter() noexcept;
		bool start();
		void stop();
		common::utile::GeoCoordinate getCurrentCoordinates();
	};
}
#endif // #MODEL_GPSADAPTER_HPP