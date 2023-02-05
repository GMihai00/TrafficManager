#include "TrafficLightStateMachine.hpp"

namespace model
{
	namespace controller
	{
		void TrafficLightStateMachine::greenLightExpireCallback()
		{

		}

		TrafficLightStateMachine::TrafficLightStateMachine(const Config& config) :
			maximumWaitingTime_(config.maxWaitingTime),
			usingLeftLane_(config.usingLeftLane),
			laneToVehicleTrackerIPAdress_(config.laneToIPAdress)
		{
			greenLightObserver_ = std::make_shared<Observer>(
				std::bind(&TrafficLightStateMachine::greenLightExpireCallback, this));
			greenLightTimer_.subscribe(greenLightObserver_);
		}

		bool TrafficLightStateMachine::isVehicleTracker(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip) const
		{
			return laneToVehicleTrackerIPAdress_.at(lane) == ip;
		}

		bool TrafficLightStateMachine::isLaneMissing(const common::utile::LANE lane) const
		{
			return !(missingLane_ != boost::none && lane == missingLane_.get());
		}

		boost::optional<common::utile::LANE> TrafficLightStateMachine::getVehicleTrackerLane(const ipc::utile::IP_ADRESS& ip)
		{
			for (const auto& entry : laneToVehicleTrackerIPAdress_)
			{
				if (entry.second == ip)
				{
					return entry.first;
				}
			}
			return boost::none;
		}

		bool TrafficLightStateMachine::isClientValid(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			std::scoped_lock lock(mutexClients_);
			if (!isVehicleTracker(lane, ip))
			{
				for (const auto& laneToIPs : clientsConnected_)
				{
					if (laneToIPs.second.find(ip) != laneToIPs.second.end())
					{
						LOG_WARN << "Client already connected";
						return false;
					}
				}
				clientsConnected_[lane].insert(ip);
			}
			return true;
		}

		bool TrafficLightStateMachine::registreClient(
			const common::utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			std::scoped_lock lock(mutexClients_);
			if (!isClientValid(lane, ip))
			{
				return false;
			}
			decreaseTimer(lane, ip);
			return true;
		}

		bool TrafficLightStateMachine::unregisterClient(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			std::scoped_lock lock(mutexClients_);
			if (clientsConnected_[lane].find(ip) == clientsConnected_[lane].end())
			{
				LOG_WARN << "Client was never connected. Nothing to do";
				return false;
			}
			clientsConnected_[lane].erase(ip);
			return true;
		}

		bool TrafficLightStateMachine::isInEmergencyState()
		{
			return !waitingEmergencyVehicles_.empty();
		}

		bool TrafficLightStateMachine::startEmergencyState(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			std::scoped_lock lock(mutexClients_);
			if (!registreClient(lane, ip))
			{
				return false;
			}

			waitingEmergencyVehicles_.push({lane, ip});
			return true;
		}

		bool TrafficLightStateMachine::endEmergencyState(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			std::scoped_lock lock(mutexClients_);
			if (!unregisterClient(lane, ip))
			{
				return false;
			}

			while (!waitingEmergencyVehicles_.empty())
			{
				auto maybeLaneIpPair = waitingEmergencyVehicles_.pop();
				if (!maybeLaneIpPair)
				{
					continue;
				}
				auto lastLane = maybeLaneIpPair.value().first;
				auto lastIp = maybeLaneIpPair.value().second;
				if (clientsConnected_[lastLane].find(lastIp) != clientsConnected_[lastLane].end())
				{
					auto valueToPushBack = maybeLaneIpPair.value();
					waitingEmergencyVehicles_.push(valueToPushBack);
					return false;
				}
			}
			
			return true;
		}

		void TrafficLightStateMachine::decreaseTimer(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			// TO DO: DECREASE TIMER BASED ON SOURCE, WILL HAVE TO THINK ABOUT AN ALGORITHM 
			if (isVehicleTracker(lane, ip))
			{

			}
			else
			{

			}
		}
	} // namespace controller
} // namespace model