#include "TrafficLightStateMachine.hpp"
#include <limits>
#include <algorithm>

namespace model
{
	namespace controller
	{
		
		TrafficLightStateMachine::TrafficLightStateMachine(const common::utile::model::JMSConfig& config, bool shouldDisplay) :
			greenLightDuration_(config.maxWaitingTime),
			regLightDuration_(120), // TO CHANGE THIS UPDATED BY ML
			usingLeftLane_(config.usingLeftLane)
		{
			missingLane_ = boost::none;
			if (config.missingLane.has_value())
				missingLane_ = config.missingLane.value();

			greeLightObserverCallback_ = std::bind(&TrafficLightStateMachine::greenLightExpireCallback, this);
			greenLightObserver_ = std::make_shared<Observer>(greeLightObserverCallback_);
			greenLightTimer_.subscribe(greenLightObserver_);
			greenLightTimer_.unfreezeTimer();

			for (uint8_t laneNr = 0; laneNr < 4; laneNr++)
			{
				common::utile::LANE lane = (common::utile::LANE)laneNr;
				if (!isLaneMissing(lane))
				{
					auto timer = std::make_shared<Timer>();
					timer->unfreezeTimer();
					laneToTimerMap_[lane] = timer;
				}
			}

			if (shouldDisplay)
			{
				windowManager_ = std::make_shared<GLFWWindowManager>();
			}
		}

		bool TrafficLightStateMachine::isUsingLeftLane()
		{
			return  usingLeftLane_;
		}
		bool TrafficLightStateMachine::isVehicleTracker(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip) const
		{
			return laneToVehicleTrackerIPAdress_.at(lane) == ip;
		}

		bool TrafficLightStateMachine::isLaneMissing(const common::utile::LANE lane) const
		{
			return missingLane_ != boost::none && lane == missingLane_.get();
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

		bool TrafficLightStateMachine::registerClient(
			const common::utile::LANE lane, ipc::utile::IP_ADRESS ip, bool leftLane)
		{
			std::scoped_lock lock(mutexClients_);
			if (!isClientValid(lane, ip))
			{
				return false;
			}
			if (leftLane == usingLeftLane_)
			{
				decreaseTimer(lane, ip);
			}
			else
			{
				// update NR OF CARS THAT PASSED JUNCTION
			}

			return true;
		}

		bool TrafficLightStateMachine::unregisterClient(ipc::utile::IP_ADRESS ip)
		{
			// crapa de aici somehow
			std::scoped_lock lock(mutexClients_);
			std::optional<common::utile::LANE> corespondingLane = {};
			for (const auto& [lane, clientsConnected] : clientsConnected_)
			{
				if (clientsConnected.find(ip) != clientsConnected.end())
				{
					corespondingLane = lane;
					break;
				}
			}

			if (!corespondingLane.has_value()) 
			{
				LOG_WARN << "Client was never connected. Nothing to do";
				return false;
			}

			clientsConnected_[corespondingLane.value()].erase(ip);
			// HERE I SHOULD UPDATE TIMER AS WELL
			return true;
		}

		bool TrafficLightStateMachine::isInEmergencyState()
		{
			return !waitingEmergencyVehicles_.empty();
		}

		bool TrafficLightStateMachine::startEmergencyState(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			//std::scoped_lock lock(mutexClients_);
			if (!registerClient(lane, ip, usingLeftLane_))
			{
				return false;
			}

			waitingEmergencyVehicles_.push({lane, ip});
			return true;
		}

		bool TrafficLightStateMachine::endEmergencyState(ipc::utile::IP_ADRESS ip)
		{
			//std::scoped_lock lock(mutexClients_);
			if (!unregisterClient(ip))
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
				auto& lastIp = maybeLaneIpPair.value().second;
				if (clientsConnected_[lastLane].find(lastIp) != clientsConnected_[lastLane].end())
				{
					auto& valueToPushBack = maybeLaneIpPair.value();
					waitingEmergencyVehicles_.push(valueToPushBack);
					return false;
				}
			}
			
			return true;
		}

		uint16_t TrafficLightStateMachine::calculateTimeDecrease(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
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
			for (const auto& chrlane : lanes)
			{
				auto lane = common::utile::CharToLane(chrlane);
				if (!lane.has_value())
					continue;

				if (auto timer = laneToTimerMap_.find(lane.value()); timer != laneToTimerMap_.end() && (timer->second))
					(timer->second)->freezeTimer();
			}
		}

		void TrafficLightStateMachine::resetTimers(const std::string lanes)
		{
			for (const auto& chrlane : lanes)
			{
				auto lane = common::utile::CharToLane(chrlane);
				if (!lane.has_value())
					continue;

				if (auto timer = laneToTimerMap_.find(lane.value()); timer != laneToTimerMap_.end() && (timer->second))
					(timer->second)->resetTimer(regLightDuration_);
			}
		}

		void TrafficLightStateMachine::decreaseTimer(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip)
		{
			if (auto timer = laneToTimerMap_.find(lane); timer != laneToTimerMap_.end() && (timer->second))
				(timer->second)->decreaseTimer(calculateTimeDecrease(lane, ip));
		}

		void TrafficLightStateMachine::updateGreenLightDuration()
		{
			std::scoped_lock lock(mutexClients_);
			// TO DO: THIS WILL BE THE ML PART 
		}

		void TrafficLightStateMachine::queueNextStatesWaiting()
		{
			std::string stateTimer[] = {"", ""};
			std::time_t expireTime[] = { std::numeric_limits<std::time_t>::max(), std::numeric_limits<std::time_t>::max() };
			for (uint8_t laneNr = 0; laneNr < 4; laneNr++)
			{
				
				common::utile::LANE lane = (common::utile::LANE)laneNr;
				if (auto timer = laneToTimerMap_.find(lane); timer != laneToTimerMap_.end() && (timer->second) && (timer->second)->hasExpired())
				{
					auto laneName = LaneToChar(common::utile::LANE(lane));
					if (laneName.has_value())
					{
						int poz = 0;
						if (lane == common::utile::LANE::E || lane == common::utile::LANE::W)
							poz = 1;

						stateTimer[poz].push_back(laneName.value());
						auto timerExpireTime = (timer->second)->getExpirationTime();
						expireTime[poz] = std::min(expireTime[poz], timerExpireTime);
					}
				}
			}
			
			if (stateTimer[0].size() != 0 && stateTimer[1].size() != 0)
			{
				if (expireTime[0] < expireTime[1])
				{
					std::swap(stateTimer[0], stateTimer[1]);
				}
			}

			if (stateTimer[0].size() != 0)
			{
				jumpTransitionQueue_.push(JumpTransition(stateTimer[0]));
			}
			if (stateTimer[1].size() != 0)
			{
				jumpTransitionQueue_.push(JumpTransition(stateTimer[1]));
			}
		}

		void TrafficLightStateMachine::updateWindowWithNewTrafficState(const std::string& transitioName)
		{
			if (!windowManager_)
				return;

			if (transitioName == "EW") { windowManager_->changeTrafficLights({ LANE::E, LANE::W }); }
			else if (transitioName == "N") { windowManager_->changeTrafficLights({ LANE::N }); }
			else if (transitioName == "S") { windowManager_->changeTrafficLights({ LANE::S }); }
			else if (transitioName == "NS") { windowManager_->changeTrafficLights({ LANE::N, LANE::S }); }
			else if (transitioName == "E") { windowManager_->changeTrafficLights({ LANE::E }); }
			else if (transitioName == "W") { windowManager_->changeTrafficLights({ LANE::W }); }
		}

		void TrafficLightStateMachine::updateTrafficState()
		{
			queueNextStatesWaiting();
			if (!jumpTransitionQueue_.empty())
			{
				const auto nextTransition = jumpTransitionQueue_.pop();
				if (!nextTransition.has_value())
				{
					LOG_ERR << "WE HAVE A BUG";
					return;
				}
				LOG_DBG << "Jumped to transition: " << nextTransition.value().nextTransitionName_;

				this->freezeTimers(nextTransition.value().nextTransitionName_);
				this->process_event(nextTransition.value());
				this->updateWindowWithNewTrafficState(nextTransition.value().nextTransitionName_);
				return;
			}

			LOG_INF << "Normal transition";

			this->process_event(NormalTransition());
			this->freezeTimers(nextNormalState_);
			this->updateWindowWithNewTrafficState(nextNormalState_);
		}

		void TrafficLightStateMachine::greenLightExpireCallback()
		{
			updateTrafficState();
			updateGreenLightDuration();
			greenLightTimer_.resetTimer(greenLightDuration_);
		}

		bool TrafficLightStateMachine::registerVehicleTrackerIpAdress(const common::utile::LANE lane, const ipc::utile::IP_ADRESS ipAdress)
		{
			std::scoped_lock lock(mutexClients_);
			if (laneToVehicleTrackerIPAdress_.find(lane) != laneToVehicleTrackerIPAdress_.end() && laneToVehicleTrackerIPAdress_.at(lane) != ipAdress)
			{
				LOG_ERR << "Camera already connect to the give lane. Please disable it";
				return false;
			}
			laneToVehicleTrackerIPAdress_[lane] = ipAdress;
			return true;
		}

	} // namespace controller
} // namespace model