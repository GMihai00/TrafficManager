#include "VehicleTrackerClient.hpp"
#include <chrono>
#include "net/ProxyReply.hpp"

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

	bool VehicleTrackerClient::setupData(ipc::net::Message<ipc::VehicleDetectionMessages> msg)
	{
		try
		{
			ipc::net::ProxyReply proxyReply{ msg };

			if (!proxyReply.isApproved()) { return false; }

			junctionIp_ = proxyReply.getServerIPAdress();
			nextJunctionCoordinates_ = proxyReply.getServerCoordinates();
			isEmergency_ = proxyReply.isEmergency();
			followedLane_ = proxyReply.getFollowedLane();
		}
		catch (const std::exception& err)
		{
			LOG_ERR << err.what();
			return false;
		}
		return true;
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
		if (!answear.has_value() || !setupData(answear.value().first.msg)) { return false; }

		return true;
	}
}