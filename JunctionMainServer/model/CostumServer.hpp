#ifndef MODEL_COSTUMSERVER_HPP
#define MODEL_COSTUMSERVER_HPP

#include <thread>
#include <mutex>
#include <chrono>

#include "net/Server.hpp"
#include "MessageTypes.hpp"
#include "GeoCoordinate.hpp"
#include "Config.hpp"
#include "controller/TrafficLightStateMachine.hpp"
#include <boost/optional/optional.hpp>

namespace model
{
	class CostumServer : ipc::net::Server<ipc::VehicleDetectionMessages>
	{
	private:
		
		//std::thread threadProcess_;
		controller::TrafficLightStateMachine trafficLightStateMachine_;
		bool usingLeftLane_;
		std::string localProxyServer_;
		std::map<uint8_t, std::string> laneToIPAdress_;
		Config config_;
	public:
		CostumServer(const uint16_t port, const Config& config);
		CostumServer(const CostumServer&) = delete;
		~CostumServer();

		virtual bool onClientConnect(
			std::shared_ptr<ipc::net::Connection<ipc::VehicleDetectionMessages>> client);
		virtual void onClientDisconnect(
			std::shared_ptr<ipc::net::Connection<ipc::VehicleDetectionMessages>> client);
		
		boost::optional<uint8_t> getMessageSourceLane(const std::string& ip);

		void handleTOMessage(
			std::shared_ptr<ipc::net::Connection<ipc::VehicleDetectionMessages>> client,
			ipc::net::Message< ipc::VehicleDetectionMessages>& msg);
		void handleVTMessage(
			std::shared_ptr<ipc::net::Connection<ipc::VehicleDetectionMessages>> client,
			ipc::net::Message< ipc::VehicleDetectionMessages>& msg);
		virtual void onMessage(
			std::shared_ptr<ipc::net::Connection<ipc::VehicleDetectionMessages>> client,
			ipc::net::Message< ipc::VehicleDetectionMessages>& msg);
		
		void postNewJunctionLocation(GeoCoordinate latitude, GeoCoordinate longitude) const;
		void deleteJunctionLocation() const;
	};
} // namespace model
#endif // #MODEL_COSTUMSERVER_HPP

