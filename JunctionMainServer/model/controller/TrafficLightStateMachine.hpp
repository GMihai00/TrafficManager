#pragma once
#ifndef MODEL_CONTROLLER_TRAFFICLIGHTSTATEMACHINE_HPP
#define MODEL_CONTROLLER_TRAFFICLIGHTSTATEMACHINE_HPP


#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <mutex>
#include <thread>
#include <functional>
#include <chrono>

#include <boost/mpl/list.hpp>
#include <boost/optional/optional.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "utile/ConfigHelpers.hpp"
#include "utile/Logger.hpp"
#include "utile/ThreadSafeQueue.hpp"
#include "utile/Timer.hpp"
#include "utile/Observer.hpp"
#include "utile/IPCDataTypes.hpp"

#include "GLFWWindowManager.hpp"

namespace sc = boost::statechart;
namespace mpl = boost::mpl;

namespace model
{
	namespace controller
	{
		using namespace common::utile;
		struct TrafficLightStateMachine;

		// EVENTS
		struct Start : sc::event<Start>
		{
		};


		struct NormalTransition : sc::event<NormalTransition>
		{
			NormalTransition() = default;
			virtual ~NormalTransition() noexcept = default;
		};

		struct JumpTransition : sc::event<JumpTransition>
		{
			std::string nextTransitionName_;
			JumpTransition() = delete;
			JumpTransition(const std::string nextTransition) :
				nextTransitionName_(nextTransition)
			{}
			virtual ~JumpTransition() noexcept = default;
		};

		struct BaseState;
		struct TrafficLightStateMachine : 
			sc::state_machine<TrafficLightStateMachine, BaseState>,
			std::enable_shared_from_this<TrafficLightStateMachine>
		{
		public:
			std::string nextNormalState_ = "EW";
		private:
			// CONFIG DATA
			uint8_t usingLeftLane_ = 0;
			boost::optional<common::utile::LANE> missingLane_ = boost::none;

			std::mutex mutexClients_;
			std::map<common::utile::LANE, ipc::utile::IP_ADRESS> laneToTrafficObserverIPAdress_;
			std::map<common::utile::LANE, ipc::utile::IP_ADRESSES> clientsConnected_;
			std::map<common::utile::LANE, common::utile::TimerPtr> laneToTimerMap_;
			common::utile::ThreadSafeQueue<JumpTransition> jumpTransitionQueue_;

			// IS TREATED AS A CLIENT SO CAN CHECK INSIDE CLIENTSCONNECTED
			common::utile::ThreadSafeQueue<std::pair<common::utile::LANE, ipc::utile::IP_ADRESS>> waitingEmergencyVehicles_;
			
			uint16_t regLightDuration_;
			uint16_t greenLightDuration_; // TO CHANGE INSIDE CONFIG MAXWAITTIME TO GREENLIGHT DURATION
			IObserverPtr greenLightObserver_;
			std::function<void()> greeLightObserverCallback_;
			common::utile::Timer greenLightTimer_;

			std::map <common::utile::LANE, uint16_t> carsWaiting_;
			std::map <common::utile::LANE, uint16_t> carsThatPassedJunction_;
			uint8_t averageWaitingCars_ = 0;
			uint16_t numberOfTransitionsTakenIntoAccount_ = 1;

			std::shared_ptr<GLFWWindowManager> windowManager_;

			LOGGER("TRAFFICLIGHT-STATEMACHINE");

			uint16_t calculateTimeDecrease(const common::utile::LANE lane, const  ipc::utile::IP_ADRESS ip, const size_t numberOfRegistrations);

			bool isInConflictScenario();

			void updateTrafficState();
			void updateTimersDuration();
			
			void updateWindowWithNewTrafficState(const std::string& transitioName);
		public:
			TrafficLightStateMachine(const common::utile::model::JMSConfig& config, const bool shouldDisplay);
			TrafficLightStateMachine(const TrafficLightStateMachine&) = delete;
			virtual ~TrafficLightStateMachine() noexcept = default;

			uint8_t isUsingLeftLane();
			// BASED ON THE LANE WE WILL DETERMINE WHAT PHAZE TO START: CAN BE EITHER II, III, VI or VII
			// as they are the only ones that allow the vehicles to move freely from one lane
			bool isVehicleTracker(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip) const;
			boost::optional<common::utile::LANE> getVehicleTrackerLane(const ipc::utile::IP_ADRESS& ip);
			bool isLaneMissing(const common::utile::LANE lane) const;

			bool isClientValid(const common::utile::LANE lane, const ipc::utile::IP_ADRESS ip);
			bool registerClient(const common::utile::LANE lane, const ipc::utile::IP_ADRESS ip, 
				const uint8_t leftLane, const size_t numberOfRegistrations = 1);
			bool unregisterClient(ipc::utile::IP_ADRESS ip);

			bool startEmergencyState(const common::utile::LANE lane, const ipc::utile::IP_ADRESS ip);
			bool isInEmergencyState();
			bool endEmergencyState(ipc::utile::IP_ADRESS ip);
			void freezeTimers(const std::string lanes);
			void resetTimers(const std::string lanes);
			void decreaseTimer(const common::utile::LANE lane, const ipc::utile::IP_ADRESS ip, const size_t numberOfRegistrations);
			void greenLightExpireCallback();
			void queueNextStatesWaiting();

			bool registerVehicleTrackerIpAdress(const common::utile::LANE lane, const ipc::utile::IP_ADRESS ipAdress);

			void updateCarCount(const std::set<common::utile::LANE> lanes);
		};


		// STATES
		struct Stopped;
		struct EWTransition;
		struct NTransition;
		struct STransition;
		struct EWTransitionCpy;
		struct NSTransition;
		struct ETransition;
		struct WTransition;
		struct NSTransitionCpy;

		struct BaseState : sc::simple_state <BaseState, TrafficLightStateMachine, Stopped>
		{
			typedef  mpl::list <sc::custom_reaction <JumpTransition> > reactions;

			BaseState()
			{
				std::cout << "BaseState\n";
			}

			virtual sc::result react(const JumpTransition& jumpTransition)
			{
				if (jumpTransition.nextTransitionName_ == "EW") { return transit<EWTransition>(); }
				else if (jumpTransition.nextTransitionName_ == "N") { return transit<NTransition>(); }
				else if (jumpTransition.nextTransitionName_ == "S") { return transit<STransition>(); }
				else if (jumpTransition.nextTransitionName_ == "NS") {return transit<NSTransition>(); }
				else if (jumpTransition.nextTransitionName_ == "E") { return transit<ETransition>(); }
				else if (jumpTransition.nextTransitionName_ == "W") { return transit<WTransition>(); }
				throw std::runtime_error("Tried to jump to undefined transition");
			}
			virtual ~BaseState() noexcept = default;
		};

		struct Stopped : sc::simple_state <Stopped, BaseState>
		{
			typedef sc::transition<NormalTransition, EWTransition> reactions;

			virtual ~Stopped() noexcept = default;

		};

	
		// STATE I/IV
		struct EWTransition : sc::simple_state <EWTransition, BaseState>
		{
			typedef  mpl::list < sc::transition<NormalTransition, NTransition> > reactions;

			EWTransition()
			{
				std::cout << "State EW started\n";
			}

			virtual ~EWTransition() noexcept
			{
				std::cout << "State ended\n";
				outermost_context().updateCarCount({ common::utile::LANE::E, common::utile::LANE::W });
				outermost_context().resetTimers("EW");
				outermost_context().nextNormalState_ = "N";
			}
		};

		// STATE II
		struct NTransition : sc::simple_state <NTransition, BaseState>
		{
			typedef  mpl::list < sc::transition<NormalTransition, STransition>> reactions;

			NTransition()
			{
				std::cout << "State N started\n";
			}

			virtual ~NTransition() noexcept
			{
				std::cout << "State ended\n";
				outermost_context().updateCarCount({ common::utile::LANE::N });
				outermost_context().resetTimers("N");
				outermost_context().nextNormalState_ = "S";
			}
		};

		// STATE III
		struct STransition : sc::simple_state <STransition, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, EWTransitionCpy>> reactions;

			STransition()
			{
				std::cout << "State S started\n";
			}

			virtual ~STransition() noexcept
			{
				std::cout << "State ended\n";
				outermost_context().updateCarCount({ common::utile::LANE::S });
				outermost_context().resetTimers("S");
				outermost_context().nextNormalState_ = "EW";
			}
		};

		// STATE I/IV
		struct EWTransitionCpy : sc::simple_state <EWTransitionCpy, BaseState>
		{
			typedef  mpl::list < sc::transition<NormalTransition, NSTransition> > reactions;

			EWTransitionCpy()
			{
				std::cout << "State EW started\n";
			}

			virtual ~EWTransitionCpy() noexcept
			{
				std::cout << "State ended\n";
				outermost_context().updateCarCount({ common::utile::LANE::E, common::utile::LANE::W });
				outermost_context().resetTimers("EW");
				outermost_context().nextNormalState_ = "NS";
			}
		};

		// STATE V/VIII
		struct NSTransition : sc::simple_state <NSTransition, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, ETransition>> reactions;

			NSTransition()
			{
				std::cout << "State NS started\n";
			}

			virtual ~NSTransition() noexcept
			{
				std::cout << "State ended\n";
				outermost_context().updateCarCount({ common::utile::LANE::N, common::utile::LANE::S });
				outermost_context().resetTimers("NS");
				outermost_context().nextNormalState_ = "E";
			}
		};

		// STATE VI
		struct ETransition : sc::simple_state <ETransition, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, WTransition>> reactions;

			ETransition()
			{
				std::cout << "State E started\n";
			}

			virtual ~ETransition() noexcept
			{
				std::cout << "State ended\n";
				outermost_context().updateCarCount({ common::utile::LANE::E });
				outermost_context().resetTimers("E");
				outermost_context().nextNormalState_ = "W";
			}
		};

		// STATE VII
		struct WTransition : sc::simple_state <WTransition, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, NSTransitionCpy>> reactions;

			WTransition()
			{
				std::cout << "State W started\n";
			}

			virtual ~WTransition() noexcept
			{
				std::cout << "State ended\n";
				outermost_context().updateCarCount({ common::utile::LANE::W });
				outermost_context().resetTimers("EW");
			}
		};

		// STATE V/VIII
		struct NSTransitionCpy : sc::simple_state <NSTransitionCpy, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, EWTransition>> reactions;

			NSTransitionCpy()
			{
				std::cout << "State NS started\n";
			}

			virtual ~NSTransitionCpy() noexcept
			{
				std::cout << "State ended\n";
				outermost_context().updateCarCount({ common::utile::LANE::N, common::utile::LANE::S });
				outermost_context().resetTimers("NS");
				outermost_context().nextNormalState_ = "EW";
			}
		};

	} // namespace controller
} // namespace model
#endif // #MODEL_CONTROLLER_TRAFFICLIGHTSTATEMACHINE_HPP
