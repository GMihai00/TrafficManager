#pragma once
#ifndef MODEL_JUNCTIONSERVER_HPP
#define MODEL_JUNCTIONSERVER_HPP

#include <thread>
#include <mutex>
#include <chrono>
#include <string>

#include <boost/optional/optional.hpp>

#include "net/Server.hpp"
#include "MessageTypes.hpp"
#include "utile/ConfigHelpers.hpp"

#include "controller/TrafficLightStateMachine.hpp"

// TO REFACTOR EVERYTHING IN HERE IT IS SO SO SOOOO WRONG
namespace model
{
	class JunctionServer : public ipc::net::Server<ipc::VehicleDetectionMessages>
	{
	private:
		
		//std::thread threadProcess_;
		controller::TrafficLightStateMachine trafficLightStateMachine_;
		std::map<common::utile::LANE, std::string> laneToKeyword_;
		LOGGER("JUNCTION-SERVER");

		boost::optional<common::utile::LANE> getMessageSourceLane(
			ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
		bool isMessageValid(
			ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg, boost::optional<common::utile::LANE> lane);
		void aproveMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
		void rejectMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg);
		boost::optional<common::utile::LANE> getLaneBasedOnKeyword(const std::string& keyword);
	public:
		JunctionServer(const common::utile::model::JMSConfig& config);
		JunctionServer(const JunctionServer&) = delete;
		virtual ~JunctionServer() noexcept = default;

		virtual bool onClientConnect(ipc::utile::ConnectionPtr client) override;
		virtual void onClientDisconnect(ipc::utile::ConnectionPtr client) override;
		virtual void onMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg) override;
		void handleMessage(ipc::utile::ConnectionPtr client, ipc::utile::VehicleDetectionMessage& msg, common::utile::LANE lane);
	};
} // namespace model
#endif // #MODEL_JUNCTIONSERVER_HPP

