#include "TrafficLightStateMachine.hpp"

namespace model
{
	namespace controller
	{

		TrafficLightStateMachine::TrafficLightStateMachine(const uint16_t maximumWaitingTime, const boost::optional<uint8_t>& missingLane) :
			maximumWaitingTime_(maximumWaitingTime),
			missingLane_(missingLane)
		{

		}

		bool TrafficLightStateMachine::isLaneMissing(const uint8_t lane)
		{
			return (this->missingLane_ == lane);
		}

		bool TrafficLightStateMachine::startEmergencyState(const uint32_t connectionId, const uint8_t lane)
		{
			std::scoped_lock lock(mutexQueue_);
			if (emergencyVehiclesWaiting_.find(connectionId) != emergencyVehiclesWaiting_.end())
			{
				LOG_DBG << " Emergency vehicle already queued";
				return false;
			}

			if (!queuedEmergencyVehicles_.empty())
			{
				LOG_DBG << " Already in emergency state";
				return false;
			}

			queuedEmergencyVehicles_.push({connectionId, lane});
			emergencyVehiclesWaiting_.insert(connectionId);
			return true;
		}

		bool TrafficLightStateMachine::isInEmergencyState()
		{
			std::scoped_lock lock(mutexQueue_);
			return !queuedEmergencyVehicles_.empty();
		}

		bool TrafficLightStateMachine::endEmergencyState(const uint32_t connectionId, const uint8_t lane)
		{
			std::scoped_lock lock(mutexQueue_);
			if (!queuedEmergencyVehicles_.empty())
			{
				LOG_DBG << " Was not in emergency state. Nothing to do";
				return false;
			}
			if (!(queuedEmergencyVehicles_.front().first == connectionId && queuedEmergencyVehicles_.front().second == lane))
			{
				return false;
			}

			emergencyVehiclesWaiting_.erase(queuedEmergencyVehicles_.front().first);
			queuedEmergencyVehicles_.pop();
			return true;
		}

		void TrafficLightStateMachine::registerDectedCars(const uint16_t numberOfCars, const uint8_t lane)
		{
			if (numberOfCars >= 0)
			{
				trafficLoadMap_[lane] = numberOfCars;
			}
			else
			{
				//UPDATE ML BASED ON THE DIFERENCE TO DO
				// IN HERE SHOULD BE TAKEN FROM EVERY DIRECTION
				int32_t dif = trafficLoadMap_[lane] - numberOfCars;
			}
		}

		bool TrafficLightStateMachine::registerVT(const uint32_t connectionId, const uint8_t lane, const bool incoming)
		{
			if (incoming)
			{
				if (vtsConnected_[lane].find(connectionId) != vtsConnected_[lane].end())
				{
					return false;
				}
				vtsConnected_[lane].insert(connectionId);
			}
			else
			{
				if (vtsConnected_[lane].find(connectionId) == vtsConnected_[lane].end())
				{
					return false;
				}
				vtsConnected_[lane].erase(connectionId);
			}
			return true;
		}

	} // namespace controller
} // namespace model