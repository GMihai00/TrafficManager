#include "JunctionServer.hpp"
#include <exception>

namespace model
{
	JunctionServer::JunctionServer(const common::utile::model::JMSConfig& config, bool shouldDisplay):
		ipc::net::Server<ipc::VehicleDetectionMessages>(config.serverIp, config.serverPort),
		trafficLightStateMachine_(config, shouldDisplay),
		laneToKeyword_(config.laneToKeyword)
	{
		try
		{
			keyPair_ = security::RSA::generateKeyPair();
		}
		catch (const std::exception& err)
		{
			LOG_ERR << err.what();
		}

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
			return trafficLightStateMachine_.getVehicleTrackerLane(client->getIpAdress());
		}
		case ipc::VehicleDetectionMessages::VDB:
		{
			try
			{
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
		std::optional<uint16_t> cars = std::nullopt;

		if (msg.header.type == ipc::VehicleDetectionMessages::VCDR)
		{
			auto carsDetected = 0;
			msg >> isFromLeftLane;
			msg >> carsDetected;
			if (carsDetected == 0)
			{
				LOG_ERR << "Invalid number of vehicles";
				rejectMessage(client, msg);
				return;
			}
			cars = carsDetected;
		}


		if (msg.header.hasPriority)
		{
			if (!trafficLightStateMachine_.startEmergencyState(lane, client->getIpAdress()))
			{
				rejectMessage(client, msg);
				return;
			}
		}

		if (cars.has_value())
			trafficLightStateMachine_.registerClient(lane, client->getIpAdress(), isFromLeftLane, cars.value());
		else
			trafficLightStateMachine_.registerClient(lane, client->getIpAdress(), isFromLeftLane);

		aproveMessage(client, msg);
	}

	void JunctionServer::providePublicKeyToClient(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		auto publicKey = std::dynamic_pointer_cast<security::RSA::PublicKey>(keyPair_.first);

		if (!publicKey)
		{
			LOG_ERR << "No key pair generate";
			rejectMessage(client, msg);
			return;
		}

		auto value = publicKey->getKeyNumericValues();

		ipc::net::Message<ipc::VehicleDetectionMessages> message;
		message.header.id = msg.header.id;
		message.header.type = ipc::VehicleDetectionMessages::ACK;

		message << value.first;
		message << value.second;

		messageClient(client, message);
	}

	bool JunctionServer::verifyIfSecureConnectionCanBeEstablished(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		std::string keyword;
		keyword.resize(msg.header.size);

		msg >> keyword;

		if (!keyPair_.second)
		{
			LOG_ERR << "No key pair generate";
			rejectMessage(client, msg);
			return false;
		}

		//auto decryptedKeyword = keyPair_.second->encrypt(keyword);
		auto decryptedKeyword = keyword;

		auto lane = getLaneBasedOnKeyword(decryptedKeyword);

		if (!lane.has_value())
		{
			rejectMessage(client, msg);
			return false;
		}

		if (!trafficLightStateMachine_.registerVehicleTrackerIpAdress(lane.value(), client->getIpAdress()))
		{
			rejectMessage(client, msg);
			return false;
		}

		aproveMessage(client, msg);
		return true;
	}

	void JunctionServer::onMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		switch (msg.header.type)
		{
		case ipc::VehicleDetectionMessages::ACK:
		case ipc::VehicleDetectionMessages::NACK:
		case ipc::VehicleDetectionMessages::REDIRECT:
			rejectMessage(client, msg);
			return;
		case ipc::VehicleDetectionMessages::PUBLIC_KEY_REQ:
			providePublicKeyToClient(client, msg);
			break;
		case ipc::VehicleDetectionMessages::SECURE_CONNECT:
			verifyIfSecureConnectionCanBeEstablished(client, msg);
			break;
		default:
			const auto lane = getMessageSourceLane(client, msg);
			if (!isMessageValid(client, msg, lane))
			{
				return;
			}
			handleMessage(client, msg, lane.get());
		}
	}

} // namespace model