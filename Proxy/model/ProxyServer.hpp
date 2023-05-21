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
#include "net/ProxyRedirect.hpp"
#include "net/Message.hpp"
#include "MessageTypes.hpp"
#include "db/Proxy.hpp"
#include "utile/DataTypes.hpp"
#include "utile/MessageIdProvider.hpp"
#include "utile/Logger.hpp"

namespace model
{
    using namespace common::utile;
	class ProxyServer : public ipc::net::Server<ipc::VehicleDetectionMessages>
	{
	private:
		common::db::ProxyPtr dbProxy_;
		std::unique_ptr<::utile::DBWrapper> dbWrapper_;
		LOGGER("PROXY-SERVER");

		bool isCoveredByProxy(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage msg);
		ipc::net::ProxyReply buildProxyReply(ipc::utile::VehicleDetectionMessage& msg, const common::db::JunctionPtr& junction) const;
		ipc::net::ProxyRedirect buildProxyRedirect(ipc::utile::VehicleDetectionMessage& msg, const common::db::ProxyPtr& proxy) const;
		void rejectMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);

		bool isMessageValid(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
		void handleMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
		std::optional<ipc::net::ProxyReply> getClosestJunctionReply(ipc::utile::VehicleDetectionMessage& msg);
		void redirect(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
	public:
		ProxyServer(const ipc::utile::IP_ADRESS& host, 
			const ipc::utile::PORT port,
			const common::db::ProxyPtr& dbProxy,
			const ::utile::DBConnectionData& connectionData);
		ProxyServer(const ProxyServer&) = delete;
		virtual ~ProxyServer() noexcept = default;

		virtual bool onClientConnect(ipc::utile::ConnectionPtr client) override;
		virtual void onClientDisconnect(ipc::utile::ConnectionPtr client) override;
		virtual void onMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg) override;
	};
}
#endif // #MODEL_VEHICLETRAKER_HPP

