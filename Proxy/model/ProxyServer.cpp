#include "ProxyServer.hpp"

namespace model
{
	ProxyServer::ProxyServer(const ipc::utile::PORT port) : 
		ipc::net::Server<ipc::VehicleDetectionMessages>(port)
	{
		// CONNECT TO DB
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