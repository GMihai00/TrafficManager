#pragma once
#ifndef MODEL_PROXYSERVER_HPP
#define MODEL_PROXYSERVER_HPP

#include <optional>
#include <string>
#include <thread>
#include <condition_variable>

#include "../utile/DBWrapper.hpp"

#include "net/Server.hpp"
#include "net/ProxyReply.hpp"
#include "net/Message.hpp"
#include "MessageTypes.hpp"
#include "db/Proxy.hpp"
#include "utile/DataTypes.hpp"
#include "utile/MessageIdProvider.hpp"
#include "utile/Logger.hpp"

namespace model
{
    using namespace common::utile;
	class ProxyServer : ipc::net::Server<ipc::VehicleDetectionMessages>
	{
	private:
		common::db::ProxyPtr dbProxy_;
		std::unique_ptr<utile::DBWrapper> dbWrapper_;
		LOGGER("PROXY-SERVER");
		bool isMessageValid(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
		void handleMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
		std::optional<ipc::net::ProxyReply> findClosestJunction(ipc::utile::VehicleDetectionMessage& msg);
		void redirect(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
	public:
		ProxyServer(const ipc::utile::PORT port, const uint32_t proxyId);
		ProxyServer(const ProxyServer&) = delete;
		virtual ~ProxyServer() noexcept = default;

		void increaseLoad();
		void decreaseLoad();
		virtual bool onClientConnect(ipc::utile::ConnectionPtr client);
		virtual void onClientDisconnect(ipc::utile::ConnectionPtr client);
		virtual void onMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
	};
}
#endif // #MODEL_VEHICLETRAKER_HPP

