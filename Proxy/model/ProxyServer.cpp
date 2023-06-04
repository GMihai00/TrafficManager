#include "ProxyServer.hpp"

#include <cppconn/exception.h>
#include "net/VehicleTrackerMessage.hpp"

namespace model
{
	ProxyServer::ProxyServer(const common::utile::model::proxy_config_data& config):
		ipc::net::Server<ipc::VehicleDetectionMessages>(config.ip, config.port)
	{
		auto coveredArea = std::make_shared<common::db::BoundingRect>(config.boundSW, config.boundNE);

		dbProxy_ = std::make_shared<common::db::Proxy>(config.ip, config.port, 0, coveredArea);

		auto connectionData = ::utile::DBConnectionData(config.dbServer, config.dbUsername, config.dbPassword);

		try
		{
			dbWrapper_ = std::make_unique<utile::DBWrapper>(connectionData);
			if (dbProxy_ == nullptr)
			{
				LOG_ERR << "FAILED TO GET CORRESPONDING DB PROXY";
				exit(2);
			}
		}
		catch (sql::SQLException e)
		{
			LOG_ERR << "FAILED TO CONNECT TO DB";
			exit(1);
		}
		LOG_INF << "Connected to database succesfully";
	}

	void ProxyServer::rejectMessage(
		ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		ipc::net::Message<ipc::VehicleDetectionMessages> message;
		message.header.id = msg.header.id;
		message.header.type = ipc::VehicleDetectionMessages::NACK;
		messageClient(client, message);
	}

	bool ProxyServer::onClientConnect(ipc::utile::ConnectionPtr client)
	{
		return true;
	}
	void ProxyServer::onClientDisconnect(ipc::utile::ConnectionPtr client)
	{
	}

	void ProxyServer::onMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		if (!isMessageValid(client, msg))
		{
			return;
		}
		handleMessage(client, msg);
	}

	ipc::net::ProxyReply ProxyServer::buildProxyReply(ipc::utile::VehicleDetectionMessage& msg, const common::db::JunctionPtr& junction) const
	{
		ipc::utile::VehicleDetectionMessage reply;
		reply.header.hasPriority = false;
		reply.header.id = msg.header.id;
		reply.header.type = ipc::VehicleDetectionMessages::ACK;
		reply << junction->getIpAdress() << junction->getPort();
		auto bounds = junction->getCoordinates()->getBounds();
		GeoCoordinate<DecimalCoordinate> boundSW = bounds.first;
		GeoCoordinate<DecimalCoordinate> boundNE = bounds.second;
		reply << boundSW.latitude << boundSW.longitude << boundNE.latitude << boundNE.longitude;

		return ipc::net::ProxyReply(reply);
	}

	ipc::net::ProxyRedirect ProxyServer::buildProxyRedirect(ipc::utile::VehicleDetectionMessage& msg, const common::db::ProxyPtr& proxy) const
	{
		ipc::utile::VehicleDetectionMessage reply;
		reply.header.hasPriority = false;
		reply.header.id = msg.header.id;
		reply.header.type = ipc::VehicleDetectionMessages::ACK;
		reply << proxy->getIpAdress() << proxy->getPort();

		return ipc::net::ProxyRedirect(reply);
	}

	std::optional<ipc::net::ProxyReply> ProxyServer::getClosestJunctionReply(ipc::utile::VehicleDetectionMessage& msg)
	{
		ipc::net::VehicleTrackerMessage vtMessage{msg};
		auto coordinates = vtMessage.getCoordinates();
		GeoCoordinate<DecimalCoordinate> pointA = coordinates.first;
		GeoCoordinate<DecimalCoordinate> pointB = coordinates.second;

		for (const auto& junction : dbWrapper_->getAllJunctions())
		{
			if (junction->isPassing(pointA, pointB))
			{
				return buildProxyReply(msg, junction);
			}
		}

		return {};
	}

	void ProxyServer::redirect(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		try
		{
			ipc::net::VehicleTrackerMessage vtMessage{ msg };
			auto coordinates = vtMessage.getCoordinates();
			GeoCoordinate<DecimalCoordinate> pointA = coordinates.first;
			GeoCoordinate<DecimalCoordinate> pointB = coordinates.second;

			auto proxy = dbWrapper_->findLeastLoadedProxyThatCoversLocation(pointB);
			if (!proxy)
			{
				proxy = dbWrapper_->findClosestProxyToPoint(pointB);
				if (!proxy)
				{
					LOG_ERR << "Couldn't find a proxy to redirect to, rejecting message";
					rejectMessage(client, msg);
					return;
				}
			}
			auto redirectmsg = buildProxyRedirect(msg, proxy);
			messageClient(client, redirectmsg);
		}
		catch (const std::runtime_error& )
		{
			LOG_WARN << "Invalid message sent to proxy by client with ip: " << client->getIpAdress();
		}
	}

	bool ProxyServer::isCoveredByProxy(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage msg)
	{
		try
		{
			ipc::net::VehicleTrackerMessage vtMessage{ msg };
			auto coordinates = vtMessage.getCoordinates();
			GeoCoordinate<DecimalCoordinate> pointA = coordinates.first;
			GeoCoordinate<DecimalCoordinate> pointB = coordinates.second;

			return dbProxy_->isContained(pointB);
		}
		catch (const std::runtime_error&)
		{
			LOG_WARN << "Invalid message sent to proxy by client with ip: " << client->getIpAdress();
			return false;
		}
	}

	void ProxyServer::handleMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		auto backupMsg = msg.clone();
		switch (msg.header.type)
		{
		case ipc::VehicleDetectionMessages::VDB:
			auto ownershipType = client->getOwner();
			LOG_DBG << "Ownership: " << (int)ownershipType;
			switch (ownershipType)
			{
			case ipc::net::Owner::Server:
				if (!isCoveredByProxy(client, msg))
				{
					LOG_WARN << "Rejecting message, proxy not covering the moving client area";
					rejectMessage(client, msg);
					return;
				}

				if (auto proxyReply = getClosestJunctionReply(msg); proxyReply.has_value())
				{
					messageClient(client, proxyReply.value());
				}
				else
				{
					LOG_INF << "Failed to find suitable junction, attempting to redirect";
					redirect(client, backupMsg);
				}
			default:
				break;
			}
		}
	}

	// FOR NOW HANDLING JUST VDB
	bool ProxyServer::isMessageValid(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		switch (msg.header.type)
		{
		case ipc::VehicleDetectionMessages::VDB:
			return true;
		default:
			return false;
		}
	}

}