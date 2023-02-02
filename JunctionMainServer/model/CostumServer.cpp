#include "CostumServer.hpp"

namespace model
{
	//when starting should connect to proxy send it's latitude and longitude
	//when shutting down should notify proxy that connection has been closed.

	void CostumServer::postNewJunctionLocation(GeoCoordinate latitude, GeoCoordinate longitude) const
	{
		// POST GEOCOORDINATES TO THE SERVER
		// IF FAILING JUST EXIT WITH ERROR CODE
	}

	void CostumServer::deleteJunctionLocation() const
	{
		// REMOVE COORDINATES FROM PROXY DB
	}

	CostumServer::CostumServer(const uint16_t port,const Config& config):
		ipc::net::Server<ipc::VehicleDetectionMessages>(port),
		trafficLightStateMachine_(config.maxWaitingTime, config.missingLane)
	{
		this->localProxyServer_ = config.localProxyServer;
		this->usingLeftLane_ = config.usingLeftLane;
		this->laneToIPAdress_ = config.laneToIPAdress;
		postNewJunctionLocation(config.latitude, config.longitude);
	}

	CostumServer::~CostumServer()
	{
		deleteJunctionLocation();
	}

	bool CostumServer::onClientConnect(
		std::shared_ptr<ipc::net::Connection<ipc::VehicleDetectionMessages>> client)
	{
		//NOTHING TO DO HERE, FOR PROXY WILL NEED TO SIGNAL SOMEHOW THAT WE ARE WAITING FOR COORDINATES
		//TO DETERMINE JUNCTION SERVER
		return true;
	}

	void CostumServer::onClientDisconnect(
		std::shared_ptr<ipc::net::Connection<ipc::VehicleDetectionMessages>> client)
	{
		// REMOVE FROM DB LOAD FOR PROXY
	}

	boost::optional<uint8_t> CostumServer::getMessageSourceLane(const std::string& ip)
	{
		for (const auto& entry : laneToIPAdress_)
		{
			if (entry.second == ip)
				return entry.first;
		}
		return boost::none;
	}

	void CostumServer::handleTOMessage(
		std::shared_ptr<ipc::net::Connection<ipc::VehicleDetectionMessages>> client,
		ipc::net::Message< ipc::VehicleDetectionMessages>& msg)
	{
		ipc::net::Message<ipc::VehicleDetectionMessages> message;
		message.header.id = msg.header.id;
		message.header.type = ipc::VehicleDetectionMessages::ACK;

		const auto& lane = getMessageSourceLane(client->getIpAdress());
		bool leaving = false;
		if (lane == boost::none)
		{
			LOG_WARN << "Invalid TO IP, ignoring request";
			message.header.type = ipc::VehicleDetectionMessages::NACK;
			client->send(message);
			return;
		}

		if (trafficLightStateMachine_.isLaneMissing(lane.get()))
		{
			LOG_WARN << "Lane is missing, invalid message sent!";
			message.header.type = ipc::VehicleDetectionMessages::NACK;
			client->send(message);
			return;
		}

		char direction;
		msg >> direction;
		if (!(direction == (usingLeftLane_ ? 'L' : 'R')))
		{
			leaving = true;
		}

		/*
		* 0->1; 1->0; 2->3; 3->2
		*/
		const auto oppositeLane = (lane.get() / 2) * 2 + (lane.get() + 1) % 2;
		if (msg.header.hasPriority)
		{
			if (leaving == false)
			{
				if (!trafficLightStateMachine_.startEmergencyState(client->getId(), lane.get()))
				{
					message.header.type = ipc::VehicleDetectionMessages::NACK;
				}
			}
			else
			{
				if (!trafficLightStateMachine_.endEmergencyState(client->getId(), oppositeLane))
				{
					message.header.type = ipc::VehicleDetectionMessages::NACK;
				}
			}
			client->send(message);
			return;
		}

		uint16_t numberOfCarsDetected;
		msg >> numberOfCarsDetected;

		if (leaving == false)
		{
			trafficLightStateMachine_.registerDectedCars(numberOfCarsDetected, lane.get());
		}
		else
		{
			trafficLightStateMachine_.registerDectedCars(numberOfCarsDetected, oppositeLane);
		}
		client->send(message);
	}

	void CostumServer::handleVTMessage(
		std::shared_ptr<ipc::net::Connection<ipc::VehicleDetectionMessages>> client,
		ipc::net::Message< ipc::VehicleDetectionMessages>& msg)
	{
		ipc::net::Message<ipc::VehicleDetectionMessages> message;
		message.header.id = msg.header.id;
		message.header.type = ipc::VehicleDetectionMessages::ACK;

		uint8_t lane;
		msg >> lane;
		if (trafficLightStateMachine_.isLaneMissing(lane))
		{
			LOG_WARN << "Lane is missing, invalid message sent!";
			message.header.type = ipc::VehicleDetectionMessages::NACK;
			client->send(message);
			return;
		}

		bool incoming;
		msg >> incoming;
		if (msg.header.hasPriority)
		{
			if (incoming == true)
			{
				if (!trafficLightStateMachine_.startEmergencyState(client->getId(), lane))
				{
					message.header.type = ipc::VehicleDetectionMessages::NACK;
				}
			}
			else
			{
				if (!trafficLightStateMachine_.endEmergencyState(client->getId(), lane))
				{
					message.header.type = ipc::VehicleDetectionMessages::NACK;
				}
			}
			client->send(message);
			return;
		}

		if (!trafficLightStateMachine_.registerVT(client->getId(), lane, incoming))
		{
			message.header.type = ipc::VehicleDetectionMessages::NACK;
		}

		client->send(message);
	}

	void CostumServer::onMessage(
		std::shared_ptr<ipc::net::Connection<ipc::VehicleDetectionMessages>> client,
		ipc::net::Message< ipc::VehicleDetectionMessages>& msg)
	{
		switch (msg.header.type)
		{
		case ipc::VehicleDetectionMessages::VCDR:
		{
			handleTOMessage(client, msg);
			break;
		}
		case ipc::VehicleDetectionMessages::VDB:
		{
			handleVTMessage(client, msg);
			break;
		}
		default:
			LOG_ERR << "Undefined message type";
		}
	}

} // namespace model