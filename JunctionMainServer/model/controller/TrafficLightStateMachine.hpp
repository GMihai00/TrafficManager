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
		private:
			// CONFIG DATA
			bool usingLeftLane_;
			boost::optional<common::utile::LANE> missingLane_ = boost::none;

			std::mutex mutexClients_;
			std::map<common::utile::LANE, ipc::utile::IP_ADRESS> laneToVehicleTrackerIPAdress_;
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

			LOGGER("TRAFFICLIGHT-STATEMACHINE");

			uint16_t calculateTimeDecrease(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip);
			void updateTrafficState();
			void updateGreenLightDuration();
		public:
			TrafficLightStateMachine(const common::utile::model::JMSConfig& config);
			TrafficLightStateMachine(const TrafficLightStateMachine&) = delete;
			virtual ~TrafficLightStateMachine() noexcept = default;

			bool isUsingLeftLane();
			// BASED ON THE LANE WE WILL DETERMINE WHAT PHAZE TO START: CAN BE EITHER II, III, VI or VII
			// as they are the only ones that allow the vehicles to move freely from one lane
			bool isVehicleTracker(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip) const;
			boost::optional<common::utile::LANE> getVehicleTrackerLane(const ipc::utile::IP_ADRESS& ip);
			bool isLaneMissing(const common::utile::LANE lane) const;

			bool isClientValid(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip);
			bool registerClient(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip, bool leftLane);
			bool unregisterClient(ipc::utile::IP_ADRESS ip);

			bool startEmergencyState(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip);
			bool isInEmergencyState();
			bool endEmergencyState(ipc::utile::IP_ADRESS ip);
			void freezeTimers(const std::string lanes);
			void resetTimers(const std::string lanes);
			void decreaseTimer(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip);
			void greenLightExpireCallback();
			void queueNextStatesWaiting();

			bool registerVehicleTrackerIpAdress(const common::utile::LANE lane, const ipc::utile::IP_ADRESS ipAdress);
		};


		// STATES
		struct Stopped;
		struct EWTransition;
		struct NTransition;
		struct STransition;
		struct NSTransition;
		struct ETransition;
		struct WTransition;
		// this is what causing the issue
		//https://www.boost.org/doc/libs/1_73_0/libs/statechart/doc/tutorial.html#TransitionActions
		//Caution: Any call to simple_state<>::transit<>() or simple_state<>::terminate() (see reference) will inevitably destruct the state object 
		// (similar to delete this;)! That is, code executed after any of these calls may invoke undefined behavior!
		// That's why these functions should only be called as part of a return statement.


		extern bool g_is_jump; // only workaround i could find, when calling transit I can not acces outermost_context()
		struct BaseState : sc::simple_state <BaseState, TrafficLightStateMachine, Stopped>
		{
			typedef  mpl::list <sc::custom_reaction <JumpTransition> > reactions;
			virtual sc::result react(const JumpTransition& jumpTransition)
			{
				g_is_jump = true;
				if (jumpTransition.nextTransitionName_ == "EW") { return transit<EWTransition>(); }
				if (jumpTransition.nextTransitionName_ == "N") { return transit<NTransition>(); }
				if (jumpTransition.nextTransitionName_ == "S") { return transit<STransition>(); }
				if (jumpTransition.nextTransitionName_ == "NS") {return transit<NSTransition>(); }
				if (jumpTransition.nextTransitionName_ == "E") { return transit<ETransition>(); }
				if (jumpTransition.nextTransitionName_ == "W") { return transit<WTransition>(); }
				throw std::runtime_error("Tried to jump to undefined transition");
			}
			virtual ~BaseState() noexcept = default;
		};

		struct Stopped : sc::simple_state <Stopped, BaseState>
		{
			typedef sc::transition<Start ,EWTransition> reactions;
		};

	
		// STATE I/IV
		struct EWTransition : sc::simple_state <EWTransition, BaseState>
		{
			typedef  mpl::list < sc::transition<NormalTransition, NTransition> > reactions;

			EWTransition()
			{
				if (g_is_jump == false)
					outermost_context().freezeTimers("EW");
			}

			virtual ~EWTransition()
			{
				outermost_context().resetTimers("EW");
			}
		};

		// STATE II
		struct NTransition : sc::simple_state <NTransition, BaseState>
		{
			typedef  mpl::list < sc::transition<NormalTransition, STransition>> reactions;

			NTransition()
			{
				if (g_is_jump == false)
					outermost_context().freezeTimers("N");
			}

			virtual ~NTransition()
			{
				outermost_context().resetTimers("N");
			}
		};

		// STATE III
		struct STransition : sc::simple_state <STransition, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, NSTransition>> reactions;

			STransition()
			{
				if (g_is_jump == false)
					outermost_context().freezeTimers("S");
			}

			~STransition()
			{
				outermost_context().resetTimers("S");
			}
		};

		// STATE V/VIII
		struct NSTransition : sc::simple_state <NSTransition, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, ETransition>> reactions;

			// TO BETTER DO THIS ALL OF THIS IN BASE STATE, JUST SET STRING IN HERE
			NSTransition()
			{
				if (g_is_jump == false)
					outermost_context().freezeTimers("NS");
			}

			virtual ~NSTransition()
			{
				outermost_context().resetTimers("NS");
			}
		};

		// STATE VI
		struct ETransition : sc::simple_state <ETransition, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, WTransition>> reactions;

			ETransition()
			{
				if (g_is_jump == false)
					outermost_context().freezeTimers("E");
			}

			virtual ~ETransition()
			{
				outermost_context().resetTimers("E");

			}
		};

		// STATE VII
		struct WTransition : sc::simple_state <WTransition, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, EWTransition>> reactions;

			WTransition()
			{
				if (g_is_jump == false)
					outermost_context().freezeTimers("EW");
			}

			virtual ~WTransition()
			{
				outermost_context().resetTimers("EW");
			}
		};

	} // namespace controller
} // namespace model
#endif // #MODEL_CONTROLLER_TRAFFICLIGHTSTATEMACHINE_HPP
