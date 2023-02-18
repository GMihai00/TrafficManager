#pragma once
#ifndef MODEL_PROXYSERVER_HPP
#define MODEL_PROXYSERVER_HPP

#include <optional>
#include <string>
#include <thread>
#include <condition_variable>

#include "net/Server.hpp"
#include "net/ProxyReply.hpp"
#include "net/Message.hpp"
#include "MessageTypes.hpp"
#include "utile/DataTypes.hpp"
#include "utile/MessageIdProvider.hpp"
#include "utile/Logger.hpp"

namespace model
{
    using namespace common::utile;
	class ProxyServer : ipc::net::Server<ipc::VehicleDetectionMessages>
	{
	private:
		LOGGER("PROXY-SERVER");
		bool isMessageValid(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
		void handleMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
		std::optional<ipc::net::ProxyReply> findClosestJunction(ipc::utile::VehicleDetectionMessage& msg);
		void redirect(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
	public:
		ProxyServer(const ipc::utile::PORT port);
		ProxyServer(const ProxyServer&) = delete;
		virtual ~ProxyServer() noexcept = default;

		virtual bool onClientConnect(ipc::utile::ConnectionPtr client);
		virtual void onClientDisconnect(ipc::utile::ConnectionPtr client);
		virtual void onMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
	};
}
#endif // #MODEL_VEHICLETRAKER_HPP

