#ifndef MODEL_JunctionServer_HPP
#define MODEL_JunctionServer_HPP

#include <thread>
#include <mutex>
#include <chrono>

#include "net/Server.hpp"
#include "MessageTypes.hpp"
#include "Config.hpp"
#include "controller/TrafficLightStateMachine.hpp"
#include <boost/optional/optional.hpp>

// TO REFACTOR EVERYTHING IN HERE IT IS SO SO SOOOO WRONG
namespace model
{
	class JunctionServer : ipc::net::Server<ipc::VehicleDetectionMessages>
	{
	private:
		
		//std::thread threadProcess_;
		controller::TrafficLightStateMachine trafficLightStateMachine_;
		ipc::utile::IP_ADRESS localProxyServer_;
		Config config_;
		LOGGER("JUNCTION-SERVER");

		boost::optional<common::utile::LANE> getMessageSourceLane(
			ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
		bool isMessageValid(
			ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg, boost::optional<common::utile::LANE> lane);
		void aproveMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
		void rejectMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
	public:
		JunctionServer(const ipc::utile::PORT port, const Config& config);
		JunctionServer(const JunctionServer&) = delete;
		virtual ~JunctionServer() noexcept = default;

		virtual bool onClientConnect(ipc::utile::ConnectionPtr client);
		virtual void onClientDisconnect(ipc::utile::ConnectionPtr client);
		virtual void onMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
		void handleMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg, common::utile::LANE lane);
	};
} // namespace model
#endif // #MODEL_JunctionServer_HPP

