#include "TrafficLightStateMachine.hpp"

namespace model
{
	namespace controller
	{
		void TrafficLightStateMachine::greenLightExpireCallback()
		{

		}

		TrafficLightStateMachine::TrafficLightStateMachine(const uint16_t maximumWaitingTime, const boost::optional<uint8_t>& missingLane) :
			maximumWaitingTime_(maximumWaitingTime),
			missingLane_(missingLane)
		{
			greenLightObserver_ = std::make_shared<Observer>(std::bind(&greenLightExpireCallback, this));
			greenLightTimer_.subscribe(greenLightObserver_);
		}

		bool TrafficLightStateMachine::registreClient(const utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			std::scoped_lock lock(mutexClients_);
			if (clientsConnected_[lane].find(ip) != clientsConnected_[lane].end())
			{
				LOG_WARN << "Client already connected";
				return false;
			}
			clientsConnected_[lane].insert(ip);
			return true;
		}

		bool TrafficLightStateMachine::unregistreCleint(const utile::LANE lane, ipc::utile::IP_ADRESS ip)
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

		bool TrafficLightStateMachine::startEmergencyState(const utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			std::scoped_lock lock(mutexClients_);
			if (!registreClient(lane, ip))
			{
				return false;
			}

			waitingEmergencyVehicles_.push({lane, ip});
			return true;
		}

		bool TrafficLightStateMachine::endEmergencyState(const utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			std::scoped_lock lock(mutexClients_);
			if (!unregistreCleint(lane, ip))
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

	} // namespace controller
} // namespace model