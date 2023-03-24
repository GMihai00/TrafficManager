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

		struct BaseTransition
		{
			std::shared_ptr<TrafficLightStateMachine> stateMachine_;
			BaseTransition() = delete;
			BaseTransition(std::shared_ptr<TrafficLightStateMachine> stateMachine)	:
				stateMachine_(stateMachine) 
			{}
			std::shared_ptr<TrafficLightStateMachine> getContextStateMachine() const
			{
				return stateMachine_;
			}
		};

		struct NormalTransition : BaseTransition, sc::event<NormalTransition>
		{
			NormalTransition() = delete;
			NormalTransition(std::shared_ptr<TrafficLightStateMachine> stateMachine) :
				BaseTransition(stateMachine)
			{}
		};

		struct JumpTransition : BaseTransition, sc::event<JumpTransition>
		{
			std::string nextTransition_;
			JumpTransition() = delete;
			JumpTransition(const std::string nextTransition, std::shared_ptr<TrafficLightStateMachine> stateMachine) :
				nextTransition_(nextTransition),
				BaseTransition(stateMachine)
			{}
		};

		struct BaseState;
		struct TrafficLightStateMachine : 
			sc::state_machine<TrafficLightStateMachine, BaseState>,
			std::enable_shared_from_this<TrafficLightStateMachine>
		{
		private:
			// CONFIG DATA
			bool usingLeftLane_;
			boost::optional<common::utile::LANE> missingLane_;

			std::mutex mutexClients_;
			std::map<common::utile::LANE, ipc::utile::IP_ADRESS> laneToVehicleTrackerIPAdress_;
			std::map<common::utile::LANE, ipc::utile::IP_ADRESSES> clientsConnected_;
			std::map<common::utile::LANE, common::utile::Timer> laneToTimerMap_;
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

		struct ContextContainer
		{
			std::shared_ptr<TrafficLightStateMachine> stateMachine_;
		};

		// STATES
		struct Stopped;
		struct EWTransition;
		struct NTransition;
		struct STransition;
		struct NSTransition;
		struct ETransition;
		struct WTransition;
		struct BaseState : sc::simple_state <BaseState, TrafficLightStateMachine, Stopped>
		{
			typedef  mpl::list <sc::custom_reaction <JumpTransition> > reactions;
			virtual sc::result react(const JumpTransition& jumpTransition)
			{
				if (jumpTransition.nextTransition_ == "EW") { return transit<EWTransition>(); }
				if (jumpTransition.nextTransition_ == "N") { return transit<NTransition>(); }
				if (jumpTransition.nextTransition_ == "S") { return transit<STransition>(); }
				if (jumpTransition.nextTransition_ == "NS") { return transit<NSTransition>(); }
				if (jumpTransition.nextTransition_ == "E") { return transit<ETransition>(); }
				if (jumpTransition.nextTransition_ == "W") { return transit<WTransition>(); }
				throw std::runtime_error("Tried to jump to undefined transition");
			}
			virtual ~BaseState() noexcept = default;
		};

		struct Stopped : sc::simple_state <Stopped, BaseState>
		{
			typedef sc::transition<Start ,EWTransition> reactions;
		};

	
		// STATE I/IV
		struct EWTransition : ContextContainer, sc::simple_state <EWTransition, BaseState>
		{
			typedef  mpl::list < sc::transition<NormalTransition, NTransition> > reactions;

			EWTransition()
			{
				auto transitionBase = dynamic_cast<const BaseTransition*> (
					sc::simple_state <EWTransition, BaseState>::triggering_event());
				if (transitionBase)
				{
					stateMachine_ = transitionBase->getContextStateMachine();
				}
				if (stateMachine_)
				{
					stateMachine_->freezeTimers("EW");
				}
			}

			~EWTransition()
			{
				if (stateMachine_)
				{
					stateMachine_->resetTimers("EW");
				}
			}
		};

		// STATE II
		struct NTransition : ContextContainer, sc::simple_state <NTransition, BaseState>
		{
			typedef  mpl::list < sc::transition<NormalTransition, STransition>> reactions;

			NTransition()
			{
				auto transitionBase = dynamic_cast<const BaseTransition*> (
					sc::simple_state <NTransition, BaseState>::triggering_event());
				if (transitionBase)
				{
					stateMachine_ = transitionBase->getContextStateMachine();
				}
				if (stateMachine_)
				{
					stateMachine_->freezeTimers("N");
				}
			}

			~NTransition()
			{
				if (stateMachine_)
					stateMachine_->resetTimers("N");
			}
		};

		// STATE III
		struct STransition : ContextContainer, sc::simple_state <STransition, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, NSTransition>> reactions;

			STransition()
			{
				auto transitionBase = dynamic_cast<const BaseTransition*> (
					sc::simple_state <STransition, BaseState>::triggering_event());
				if (transitionBase)
				{
					stateMachine_ = transitionBase->getContextStateMachine();
				}
				if (stateMachine_)
				{
					stateMachine_->freezeTimers("S");
				}
			}

			~STransition()
			{
				if (stateMachine_)
				{
					stateMachine_->resetTimers("S");
				}
			}
		};

		// STATE V/VIII
		struct NSTransition : ContextContainer, sc::simple_state <NSTransition, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, ETransition>> reactions;

			// TO BETTER DO THIS ALL OF THIS IN BASE STATE, JUST SET STRING IN HERE
			NSTransition()
			{
				auto transitionBase = dynamic_cast<const BaseTransition*> (
					sc::simple_state <NSTransition, BaseState>::triggering_event());
				if (transitionBase)
				{
					stateMachine_ = transitionBase->getContextStateMachine();
				}
				if (stateMachine_)
				{
					stateMachine_->freezeTimers("NS");
				}
			}

			~NSTransition()
			{
				if (stateMachine_)
				{
					stateMachine_->resetTimers("NS");
				}
			}
		};

		// STATE VI
		struct ETransition : ContextContainer, sc::simple_state <ETransition, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, WTransition>> reactions;

			ETransition()
			{
				auto transitionBase = dynamic_cast<const BaseTransition*> (
					sc::simple_state <ETransition, BaseState>::triggering_event());
				if (transitionBase)
				{
					stateMachine_ = transitionBase->getContextStateMachine();
				}
				if (stateMachine_)
				{
					stateMachine_->freezeTimers("E");
				}
			}

			~ETransition()
			{
				if (stateMachine_)
				{
					stateMachine_->resetTimers("E");
				}
			}
		};

		// STATE VII
		struct WTransition : ContextContainer, sc::simple_state <WTransition, BaseState>
		{
			typedef  mpl::list <sc::transition<NormalTransition, EWTransition>> reactions;

			WTransition()
			{
				auto transitionBase = dynamic_cast<const BaseTransition*> (
					sc::simple_state <WTransition, BaseState>::triggering_event());
				if (transitionBase)
				{
					stateMachine_ = transitionBase->getContextStateMachine();
				}
				if (stateMachine_)
				{
					stateMachine_->freezeTimers("EW");
				}
			}

			~WTransition()
			{
				if (stateMachine_)
				{
					stateMachine_->resetTimers("EW");
				}
			}
		};

	} // namespace controller
} // namespace model
#endif // #MODEL_CONTROLLER_TRAFFICLIGHTSTATEMACHINE_HPP
