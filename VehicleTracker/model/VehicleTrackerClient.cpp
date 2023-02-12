#include "VehicleTrackerClient.hpp"
#include <chrono>

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

	std::optional<ipc::utile::IP_ADRESS> VehicleTrackerClient::getIpFromMessage(ipc::net::Message<ipc::VehicleDetectionMessages>& message)
	{
		// GET IP ADRESS OF SERVER
		// Normal IPV4 255.255.253.0
		// Server will send instead 255255253000
		ipc::utile::IP_ADRESS ipAdress;
		std::string subNr;
		int cntSubNr = 0;
		char chr = 0;
		do
		{
			message >> chr;
			subNr.push_back(chr);
			if (subNr.size() == 3)
			{
				try
				{
					int nr = std::stoi(subNr);
					if (!(nr >= 0 && nr <= 255)) { return {}; }

					subNr = std::to_string(nr);
					subNr.push_back('.');
					ipAdress += subNr;
					subNr.clear();
					cntSubNr++;
				}
				catch (...)
				{
					return {};
				}
			}
		} while (chr);

		return ((cntSubNr == 4) ? std::optional<ipc::utile::IP_ADRESS>{ipAdress} : std::nullopt);
	}

	// SEND 2 CONSECUTIVE COORDINATES
	// RECIEVE JUNCTION IP ADRESS, LANE, JUNCTION COORDINATES IF IN AREA OF COVERAGE 
	// TO DO: FOR SECURITY PROXY WILL SEND A CRYPTED MESSAGE LOOK AT KERBEROS PROTOCOL
	// https://www.youtube.com/watch?v=5N242XcKAsM
	bool VehicleTrackerClient::queryProxyServer()
	{
		if (!isConnected()) { return false; }

		auto start = gpsAdapter_.getCurrentCoordinates();
		auto current = gpsAdapter_.getCurrentCoordinates();
		if (start == current) { return false; }

		auto msgId = messageIdProvider_.provideId(ipc::VehicleDetectionMessages::VDB);
		ipc::net::Message<ipc::VehicleDetectionMessages> message;
		message.header.id = msgId;
		message.header.type = ipc::VehicleDetectionMessages::VDB;
		message << start.latitude << start.longitude << current.latitude << current.longitude;
		send(message);

		// WAIT FOR RESPONSE
		while (!answearRecieved()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }

		// GET MESSAGE AND CHECK TYPE
		auto answear = getLastUnreadAnswear();
		if (!answear.has_value()) { return false; }

		message = answear.value().first.msg;
		if (message.header.id != msgId || message.header.type != ipc::VehicleDetectionMessages::ACK) { return false; }

		auto maybeIpAdress = getIpFromMessage(message);
		if (!maybeIpAdress.has_value()) { return false; }

		isEmergencyVehicle_ = answear.value().second;
		junctionIp_ = maybeIpAdress.value();
		return true;
	}
}