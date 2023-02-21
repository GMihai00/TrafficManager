#include "ProxyServer.hpp"

#include <cppconn/exception.h>
namespace model
{
	ProxyServer::ProxyServer(const ipc::utile::PORT port, const uint32_t proxyId) :
		ipc::net::Server<ipc::VehicleDetectionMessages>(port)
	{
		try
		{
			dbWrapper_ = std::make_unique<utile::DBWrapper>("", "", "");
			dbProxy_ = dbWrapper_->findProxyById(proxyId);
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
	}

	void ProxyServer::increaseLoad()
	{
		dbWrapper_->updateProxyLoad(dbProxy_, true);
	}

	void ProxyServer::decreaseLoad()
	{
		dbWrapper_->updateProxyLoad(dbProxy_, false);
	}

	bool ProxyServer::onClientConnect(ipc::utile::ConnectionPtr client)
	{
		increaseLoad();
		return true;
	}
	void ProxyServer::onClientDisconnect(ipc::utile::ConnectionPtr client)
	{
		decreaseLoad();
	}

	void ProxyServer::onMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		if (!isMessageValid(client, msg))
		{
			return;
		}
		handleMessage(client, msg);
	}

	std::optional<ipc::net::ProxyReply> ProxyServer::findClosestJunction(ipc::utile::VehicleDetectionMessage& msg)
	{
		std::optional<ipc::net::ProxyReply> rez = {};

		return rez;
	}

	void ProxyServer::redirect(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{

	}

	void ProxyServer::handleMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		switch (msg.header.type)
		{
		case ipc::VehicleDetectionMessages::VCDR:
			auto ownershipType = client->getOwner();
			switch (ownershipType)
			{
			case ipc::net::Owner::Server:
				break;
			case ipc::net::Owner::Proxy:
				// TO THINK ABOUT THIS HOW DO I GET THE ANSWEAR FROM THE REDIRECTED MESSAGE BACK TO THE CLIENT
				break;
			case ipc::net::Owner::Client:
				if (auto proxyReply = findClosestJunction(msg); proxyReply.has_value())
				{
					client->send(proxyReply.value());
				}
				else
				{
					redirect(client, msg);
				}
				break;
			default:
				break;
			}
		}
	}

	// FOR NOW HANDLING JUST VCDR
	bool ProxyServer::isMessageValid(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg)
	{
		switch (msg.header.type)
		{
		case ipc::VehicleDetectionMessages::VCDR:
			return true;
		default:
			return false;
		}
	}

}