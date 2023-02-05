#include "CostumServer.hpp"

namespace model
{
	CostumServer::CostumServer(const ipc::utile::PORT port,const Config& config):
		ipc::net::Server<ipc::VehicleDetectionMessages>(port),
		trafficLightStateMachine_(config)
	{
		this->localProxyServer_ = config.localProxyServer;
		trafficLightStateMachine_.initiate();
	}

	bool CostumServer::onClientConnect(ipc::utile::ConnectionPtr client)
	{
		// TO THINK ABOUT THIS
		return true;
	}

	void CostumServer::onClientDisconnect(ipc::utile::ConnectionPtr client)
	{
		// TO THINK ABOUT THIS
	}


	void CostumServer::aproveMessage(
		ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		ipc::net::Message<ipc::VehicleDetectionMessages> message;
		message.header.id = msg.header.id;
		message.header.type = ipc::VehicleDetectionMessages::ACK;
		client->send(message);
	}

	void CostumServer::rejectMessage(
		ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		LOG_WARN << "Rejecting invalid message";
		ipc::net::Message<ipc::VehicleDetectionMessages> message;
		message.header.id = msg.header.id;
		message.header.type = ipc::VehicleDetectionMessages::NACK;
		client->send(message);
	}

	boost::optional<common::utile::LANE> CostumServer::getMessageSourceLane(
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

	bool CostumServer::isMessageValid(
		ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg, boost::optional<common::utile::LANE> lane)
	{
		if (lane == boost::none || trafficLightStateMachine_.isLaneMissing(lane.get()))
		{
			rejectMessage(client, msg);
			return false;
		}
		return true;
	}

	void CostumServer::handleMessage(
		ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg, common::utile::LANE lane)
	{
		bool leaving;
		msg >> leaving;
		if (msg.header.hasPriority)
		{
			if (leaving == false)
			{
				if (!trafficLightStateMachine_.startEmergencyState(lane, client->getIpAdress()))
				{
					rejectMessage(client, msg);
					return;
				}
			}
			else
			{
				if (!trafficLightStateMachine_.endEmergencyState(lane, client->getIpAdress()))
				{
					rejectMessage(client, msg);
					return;
				}
			}
		}
		trafficLightStateMachine_.registreClient(lane, client->getIpAdress());
		aproveMessage(client, msg);
	}

	void CostumServer::onMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		const auto lane = getMessageSourceLane(client, msg);
		if (!isMessageValid(client, msg, lane))
		{
			return;
		}
		handleMessage(client, msg, lane.get());
	}

} // namespace model