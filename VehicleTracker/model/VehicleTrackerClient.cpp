#include "VehicleTrackerClient.hpp"

namespace model
{
	VehicleTrackerClient::VehicleTrackerClient(std::istream& inputStream) :
		ipc::net::Client<ipc::VehicleDetectionMessages>(),
		gpsAdapter_(inputStream)
	{

	}

	VehicleTrackerClient::~VehicleTrackerClient()
	{

	}

	// IF NOT JUNCTION NEARBY DOES NOTHING
	// ALSO IF COORDINATES FROM 2 SEPARATE FRAMES ARE THE SAME SHOULD DO NOTHING AS WELL
	bool VehicleTrackerClient::queryProxyServer()
	{
		auto firstCoordinates = gpsAdapter_.getCurrentCoordinates();
		auto secondCoordinates = gpsAdapter_.getCurrentCoordinates();
		if (firstCoordinates == secondCoordinates) { return false; }
		// BASED ON THIS I SHOULD BE ABLE TO DETERMINE DIRECTION
		// HOW SHOULD I MEASURE DIRECTION?
	
	}
}