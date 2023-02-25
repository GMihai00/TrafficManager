#include "JunctionServer.hpp"

namespace model
{
	JunctionServer::JunctionServer(const ipc::utile::PORT port,const Config& config):
		ipc::net::Server<ipc::VehicleDetectionMessages>(port),
		trafficLightStateMachine_(config)
	{
		this->localProxyServer_ = config.localProxyServer;
		trafficLightStateMachine_.initiate();
	}

	bool JunctionServer::onClientConnect(ipc::utile::ConnectionPtr client)
	{
		// TO THINK ABOUT THIS
		return true;
	}

	void JunctionServer::onClientDisconnect(ipc::utile::ConnectionPtr client)
	{
		if (!trafficLightStateMachine_.endEmergencyState(client->getIpAdress()))
		{
			trafficLightStateMachine_.unregisterClient(client->getIpAdress());
		}
	}


	void JunctionServer::aproveMessage(
		ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		ipc::net::Message<ipc::VehicleDetectionMessages> message;
		message.header.id = msg.header.id;
		message.header.type = ipc::VehicleDetectionMessages::ACK;
		messageClient(client, message);
	}

	void JunctionServer::rejectMessage(
		ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		LOG_WARN << "Rejecting invalid message";
		ipc::net::Message<ipc::VehicleDetectionMessages> message;
		message.header.id = msg.header.id;
		message.header.type = ipc::VehicleDetectionMessages::NACK;
		messageClient(client ,message);
	}

	boost::optional<common::utile::LANE> JunctionServer::getMessageSourceLane(
		ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		switch (msg.header.type)
		{
		case ipc::VehicleDetectionMessages::VCDR:
		{
			return trafficLightStateMachine_.getVehicleTrackerLane(client->getIpAdress());
		}
		case ipc::VehicleDetectionMessages::VDB:
		{
			try
			{
				// GOT BY CLIENT FROM PROXY
				common::utile::LANE lane;
				msg >> lane;
				return lane;
			}
			catch (...)
			{
				return boost::none;
			}
		}
		default:
			return boost::none;
		}
	}

	bool JunctionServer::isMessageValid(
		ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg, boost::optional<common::utile::LANE> lane)
	{
		if (lane == boost::none || trafficLightStateMachine_.isLaneMissing(lane.get()))
		{
			rejectMessage(client, msg);
			return false;
		}
		return true;
	}

	//  THIS METHOD NEEDS TO BE CHANGED
	void JunctionServer::handleMessage(
		ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg, common::utile::LANE lane)
	{
		if (msg.header.hasPriority)
		{
			if (!trafficLightStateMachine_.startEmergencyState(lane, client->getIpAdress()))
			{
				rejectMessage(client, msg);
				return;
			}
		}
		trafficLightStateMachine_.registerClient(lane, client->getIpAdress());
		aproveMessage(client, msg);
	}

	void JunctionServer::onMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		const auto lane = getMessageSourceLane(client, msg);
		if (!isMessageValid(client, msg, lane))
		{
			return;
		}
		handleMessage(client, msg, lane.get());
	}

} // namespace model