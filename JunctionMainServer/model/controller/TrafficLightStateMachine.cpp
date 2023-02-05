#include "TrafficLightStateMachine.hpp"
#include "utile/Helpers.hpp"
#include <algorithm>

namespace model
{
	namespace controller
	{

		TrafficLightStateMachine::TrafficLightStateMachine(const Config& config) :
			greenLightDuration_(config.maxWaitingTime),
			regLightDuration_(120), // TO CHANGE THIS UPDATED BY ML
			usingLeftLane_(config.usingLeftLane),
			laneToVehicleTrackerIPAdress_(config.laneToIPAdress)
		{
			// FOR THE REALLY BAD WORKAROUND TO BE REMOVED
			Transition::stateMachine_ = shared_from_this();
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

		uint16_t TrafficLightStateMachine::calculateTimeDecrease(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			std::scoped_lock lock(mutexClients_);
			// TO DO: CALCULATE TIME BASED ON SOURCE, WILL HAVE TO THINK ABOUT AN ALGORITHM 
			uint16_t rez = 0;
			if (isVehicleTracker(lane, ip))
			{

			}
			else
			{

			}
			return rez;
		}

		void TrafficLightStateMachine::freezeTimers(const std::string lanes)
		{
			for (const auto& lane : lanes)
			{
				laneToTimerMap_.at(common::utile::CharToLane(lane).value()).freezeTimer();
			}
		}

		void TrafficLightStateMachine::resetTimers(const std::string lanes)
		{
			for (const auto& lane : lanes)
			{
				laneToTimerMap_.at(common::utile::CharToLane(lane).value()).resetTimer(regLightDuration_);
			}
		}

		void TrafficLightStateMachine::decreaseTimer(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			std::scoped_lock lock(mutexClients_);
			laneToTimerMap_.at(lane).decreaseTimer(calculateTimeDecrease(lane, ip));
		}

		void TrafficLightStateMachine::updateGreenLightDuration()
		{
			std::scoped_lock lock(mutexClients_);
			// TO DO: THIS WILL BE THE ML PART 
		}

		void TrafficLightStateMachine::queueNextStatesWaiting()
		{
			// STUPIDEST IMPLEMENTATION I KNOW TO REFACTOR THIS
			bool expiredN = false;
			bool expiredS = false;
			std::time_t expireTimeSN = LONG_MAX;
			bool expiredE = false;
			bool expiredW = false;
			std::time_t expireTimeEW = LONG_MAX;

			if (laneToTimerMap_.at(common::utile::LANE::N).hasExpired())
			{
				expiredN = true;
				expireTimeSN = std::min(expireTimeSN, laneToTimerMap_.at(common::utile::LANE::N).getExpirationTime());
			}

			if (laneToTimerMap_.at(common::utile::LANE::S).hasExpired())
			{
				expiredS = true;
				expireTimeSN = std::min(expireTimeSN, laneToTimerMap_.at(common::utile::LANE::S).getExpirationTime());
			}

			if (laneToTimerMap_.at(common::utile::LANE::E).hasExpired())
			{
				expiredE = true;
				expireTimeEW = std::min(expireTimeSN, laneToTimerMap_.at(common::utile::LANE::E).getExpirationTime());
			}

			if (laneToTimerMap_.at(common::utile::LANE::W).hasExpired())
			{
				expiredW = true;
				expireTimeEW = std::min(expireTimeSN, laneToTimerMap_.at(common::utile::LANE::W).getExpirationTime());
			}

			if ((expiredN && expiredS) && (expiredE && expiredW))
			{
				if (expireTimeEW < expireTimeSN)
				{
					jumpTransitionQueue_.push(JumpTransition("EW", shared_from_this()));
					jumpTransitionQueue_.push(JumpTransition("SN", shared_from_this()));
				}
				else
				{
					jumpTransitionQueue_.push(JumpTransition("SN", shared_from_this()));
					jumpTransitionQueue_.push(JumpTransition("EW", shared_from_this()));
				}
				return;
			}

			if (expiredN  && (expiredE && expiredW))
			{
				if (expireTimeEW < expireTimeSN)
				{
					jumpTransitionQueue_.push(JumpTransition("EW", shared_from_this()));
					jumpTransitionQueue_.push(JumpTransition("N", shared_from_this()));
				}
				else
				{
					jumpTransitionQueue_.push(JumpTransition("N", shared_from_this()));
					jumpTransitionQueue_.push(JumpTransition("EW", shared_from_this()));
				}
				return;
			}

			if (expiredS && (expiredE && expiredW))
			{
				if (expireTimeEW < expireTimeSN)
				{
					jumpTransitionQueue_.push(JumpTransition("EW", shared_from_this()));
					jumpTransitionQueue_.push(JumpTransition("S", shared_from_this()));
				}
				else
				{
					jumpTransitionQueue_.push(JumpTransition("S", shared_from_this()));
					jumpTransitionQueue_.push(JumpTransition("EW", shared_from_this()));
				}
				return;
			}

			if ((expiredN && expiredS) && expiredE)
			{
				if (expireTimeEW < expireTimeSN)
				{
					jumpTransitionQueue_.push(JumpTransition("E", shared_from_this()));
					jumpTransitionQueue_.push(JumpTransition("NS", shared_from_this()));
				}
				else
				{
					jumpTransitionQueue_.push(JumpTransition("NS", shared_from_this()));
					jumpTransitionQueue_.push(JumpTransition("E", shared_from_this()));
				}
				return;
			}

			if ((expiredN && expiredS) && expiredW)
			{
				if (expireTimeEW < expireTimeSN)
				{
					jumpTransitionQueue_.push(JumpTransition("W", shared_from_this()));
					jumpTransitionQueue_.push(JumpTransition("NS", shared_from_this()));
				}
				else
				{
					jumpTransitionQueue_.push(JumpTransition("NS", shared_from_this()));
					jumpTransitionQueue_.push(JumpTransition("W", shared_from_this()));
				}
				return;
			}

			if (expiredN && expiredS)
			{
				jumpTransitionQueue_.push(JumpTransition("SN", shared_from_this()));
				return;
			}

			if (expiredE && expiredW)
			{
				jumpTransitionQueue_.push(JumpTransition("EW", shared_from_this()));
				return;
			}

			if (expiredE)
			{
				jumpTransitionQueue_.push(JumpTransition("E", shared_from_this()));
			}
			if (expiredS)
			{
				jumpTransitionQueue_.push(JumpTransition("S", shared_from_this()));
			}

			if (expiredN)
			{
				jumpTransitionQueue_.push(JumpTransition("N", shared_from_this()));
			}

			if (expiredW)
			{
				jumpTransitionQueue_.push(JumpTransition("W", shared_from_this()));
			}
		}

		void TrafficLightStateMachine::updateTrafficState()
		{
			std::scoped_lock lock(mutexClients_);
			queueNextStatesWaiting();
			if (!jumpTransitionQueue_.empty())
			{
				const auto nextTransition = jumpTransitionQueue_.pop();
				if (!nextTransition.has_value())
				{
					LOG_ERR << "WE HAVE A BUG";
					return;
				}
				this->process_event(nextTransition.value());
				return;
			}
			this->process_event(NormalTransition(shared_from_this()));
		}

		void TrafficLightStateMachine::greenLightExpireCallback()
		{
			updateTrafficState();
			updateGreenLightDuration();
			greenLightTimer_.resetTimer(greenLightDuration_);
		}

	} // namespace controller
} // namespace model