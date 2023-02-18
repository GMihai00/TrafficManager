#include "VehicleTrackerClient.hpp"

#include <chrono>
#include <exception>

#include "net/ProxyReply.hpp"

namespace model
{
	void VehicleTrackerClient::process()
	{
		while (true)
		{
			std::unique_lock<std::mutex> ulock(mutexProcess_);
			condVarProcess_.wait(ulock, [&] { return (shouldPause_ == false); });

			while (!shouldPause_)
			{
				if (!connect(ipc::utile::G_PROXY_IP, ipc::utile::G_PROXY_PORT))
				{
					throw std::runtime_error("Failed to communicate with proxy");
				}

				if (!queryProxy())
				{
					throw std::runtime_error("Failed to communicate with proxy");
				}
				disconnect();

				if (!connect(junctionIpAndPort_.first, junctionIpAndPort_.second))
				{
					LOG_ERR << "FAILED TO COMMUNICATE WITH JUNCTION";
					continue;
				}

				if (!notifyJunction())
				{
					LOG_ERR << "FAILED TO COMMUNICATE WITH JUNCTION";
					continue;
				}

				waitToPassJunction();

				disconnect();
			}
		}
	}

	VehicleTrackerClient::VehicleTrackerClient(std::istream& inputStream) :
		ipc::net::Client<ipc::VehicleDetectionMessages>(),
		gpsAdapter_(inputStream)
	{
		threadProcess_ = std::thread(std::bind(&VehicleTrackerClient::process, this));
	}

	VehicleTrackerClient::~VehicleTrackerClient()
	{

	}

	bool VehicleTrackerClient::start()
	{
		std::unique_lock<std::mutex> ulock(mutexProcess_);
		shouldPause_ = false;
		condVarProcess_.notify_one();
		return true;
	}

	void VehicleTrackerClient::pause()
	{
		shouldPause_ = true;
	}

	// SEND 2 CONSECUTIVE COORDINATES
	// RECIEVE JUNCTION IP ADRESS, LANE, JUNCTION COORDINATES IF IN AREA OF COVERAGE 
	// TO DO: FOR SECURITY PROXY WILL SEND A CRYPTED MESSAGE LOOK AT KERBEROS PROTOCOL
	// https://www.youtube.com/watch?v=5N242XcKAsM
	bool VehicleTrackerClient::queryProxy()
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

	bool VehicleTrackerClient::setupData(ipc::net::Message<ipc::VehicleDetectionMessages> msg)
	{
		try
		{
			ipc::net::ProxyReply proxyReply{ msg };

			if (!proxyReply.isApproved()) { return false; }

			junctionIpAndPort_ = proxyReply.getServerIPAdressAndPort();
			junctionCoordinates_ = proxyReply.getServerCoordinates();
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

	// TO DO
	bool VehicleTrackerClient::notifyJunction()
	{
		if (!isConnected())
		{
			return false;
		}
		auto msgId = messageIdProvider_.provideId(ipc::VehicleDetectionMessages::VDB);
		ipc::net::Message<ipc::VehicleDetectionMessages> message;
		message.header.id = msgId;
		message.header.type = ipc::VehicleDetectionMessages::VDB;
		message << followedLane_;
		send(message);

		return true;
	}

	// TO DO
	void VehicleTrackerClient::waitToPassJunction()
	{

	}
}