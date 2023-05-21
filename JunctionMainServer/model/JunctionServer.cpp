#include "JunctionServer.hpp"

namespace model
{
	JunctionServer::JunctionServer(const common::utile::model::JMSConfig& config, bool shouldDisplay):
		ipc::net::Server<ipc::VehicleDetectionMessages>(config.serverIp, config.serverPort),
		trafficLightStateMachine_(config, shouldDisplay),
		laneToKeyword_(config.laneToKeyword)
	{
		trafficLightStateMachine_.initiate();
	}

	bool JunctionServer::onClientConnect(ipc::utile::ConnectionPtr client)
	{
		return true;
	}

	void JunctionServer::onClientDisconnect(ipc::utile::ConnectionPtr client)
	{
		trafficLightStateMachine_.endEmergencyState(client->getIpAdress());
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

	boost::optional<common::utile::LANE> JunctionServer::getLaneBasedOnKeyword(const std::string& keyword)
	{
		for (const auto& entry : laneToKeyword_)
		{
			if (entry.second == keyword)
			{
				return entry.first;
			}
		}
		return boost::none;
	}

	boost::optional<common::utile::LANE> JunctionServer::getMessageSourceLane(
		ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		switch (msg.header.type)
		{ 
		case ipc::VehicleDetectionMessages::VCDR:
		{
			// should add a some sort of encryption layer here, maybe add public private key in here
			std::string keyword;
			keyword.resize(msg.header.size - sizeof(uint8_t));
			msg >> keyword;

			auto lane = getLaneBasedOnKeyword(keyword);
			return lane;
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
			LOG_ERR << "Invalid lane";
			rejectMessage(client, msg);
			return false;
		}
		return true;
	}

	void JunctionServer::handleMessage(
		ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg, common::utile::LANE lane)
	{
		uint8_t isFromLeftLane = trafficLightStateMachine_.isUsingLeftLane();
		if (msg.header.type == ipc::VehicleDetectionMessages::VCDR)
		{
			if (!trafficLightStateMachine_.registerVehicleTrackerIpAdress(lane, client->getIpAdress()))
			{
				rejectMessage(client, msg);
				return;
			}

			msg >> isFromLeftLane;
		}


		if (msg.header.hasPriority)
		{
			if (!trafficLightStateMachine_.startEmergencyState(lane, client->getIpAdress()))
			{
				rejectMessage(client, msg);
				return;
			}
		}
		trafficLightStateMachine_.registerClient(lane, client->getIpAdress(), isFromLeftLane);
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