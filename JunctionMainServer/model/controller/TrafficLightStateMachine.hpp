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

#include "../Config.hpp"
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

		struct Transition
		{
			std::shared_ptr<TrafficLightStateMachine> stateMachine_;
			virtual sc::result react(const JumpTransition& jumpTransition) = 0;
			virtual ~Transition() noexcept = default;
		};

		struct Stopped;
		struct EWTransition;
		struct NTransition;
		struct STransition;
		struct NSTransition;
		struct ETransition;
		struct WTransition;
		struct TrafficLightStateMachine : 
			sc::state_machine<TrafficLightStateMachine, Stopped>,
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
			uint16_t greenLightDuration_; // TO CHANGE INSIDE CONFID MAXWAITTIME TO GREENLIGHT DURATION
			IObserverPtr greenLightObserver_;
			common::utile::Timer greenLightTimer_;
			LOGGER("TRAFFICLIGHT-STATEMACHINE");

			uint16_t calculateTimeDecrease(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip);
			void updateTrafficState();
			void updateGreenLightDuration();
		public:
			TrafficLightStateMachine(const Config& config);
			TrafficLightStateMachine(const TrafficLightStateMachine&) = delete;
			virtual ~TrafficLightStateMachine() noexcept = default;

			// BASED ON THE LANE WE WILL DETERMINE WHAT PHAZE TO START: CAN BE EITHER II, III, VI or VII
			// as they are the only ones that allow the vehicles to move freely from one lane
			bool isVehicleTracker(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip) const;
			boost::optional<common::utile::LANE> getVehicleTrackerLane(const ipc::utile::IP_ADRESS& ip);
			bool isLaneMissing(const common::utile::LANE lane) const;

			bool isClientValid(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip);
			bool registreClient(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip);
			bool unregisterClient(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip);

			bool startEmergencyState(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip);
			bool isInEmergencyState();
			bool endEmergencyState(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip);

			void freezeTimers(const std::string lanes);
			void resetTimers(const std::string lanes);
			void decreaseTimer(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip);
			void greenLightExpireCallback();
			void queueNextStatesWaiting();
		};

		// STATES
		struct Stopped : sc::simple_state <Stopped, TrafficLightStateMachine>
		{
			typedef sc::transition<Start ,EWTransition> reactions;
		};

		// CONSTRUCTORS FOR ALL OF THEM DEFINED TO FREEZE AND UNFREEZE TIMERS
		// STATE I/IV
		struct EWTransition : public Transition, sc::simple_state <EWTransition, TrafficLightStateMachine>
		{
			typedef  mpl::list < 
				sc::transition<NormalTransition, NTransition>,
				sc::custom_reaction <JumpTransition> > reactions;

			sc::result react(const JumpTransition& jumpTransition)
			{
				// TO FIND A WAY TO REMOVE DUBLICATE CODE
				if (jumpTransition.nextTransition_ == "EW") { return transit<EWTransition>(); }
				if (jumpTransition.nextTransition_ == "N") { return transit<NTransition>(); }
				if (jumpTransition.nextTransition_ == "S") { return transit<STransition>(); }
				if (jumpTransition.nextTransition_ == "NS") { return transit<NSTransition>(); }
				if (jumpTransition.nextTransition_ == "E") { return transit<ETransition>(); }
				if (jumpTransition.nextTransition_ == "W") { return transit<WTransition>(); }
				throw std::runtime_error("Tried to jump to undefined transition");
			}

			EWTransition()
			{
				auto transitionBase = dynamic_cast<const BaseTransition*> (triggering_event());
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
		struct NTransition : public Transition, sc::simple_state <NTransition, TrafficLightStateMachine>
		{
			typedef  mpl::list <
				sc::transition<NormalTransition, STransition>,
				sc::custom_reaction <JumpTransition> > reactions;

			sc::result react(const JumpTransition& jumpTransition)
			{
				// TO FIND A WAY TO REMOVE DUBLICATE CODE
				if (jumpTransition.nextTransition_ == "EW") { return transit<EWTransition>(); }
				if (jumpTransition.nextTransition_ == "N") { return transit<NTransition>(); }
				if (jumpTransition.nextTransition_ == "S") { return transit<STransition>(); }
				if (jumpTransition.nextTransition_ == "NS") { return transit<NSTransition>(); }
				if (jumpTransition.nextTransition_ == "E") { return transit<ETransition>(); }
				if (jumpTransition.nextTransition_ == "W") { return transit<WTransition>(); }
				throw std::runtime_error("Tried to jump to undefined transition");
			}

			NTransition()
			{
				auto transitionBase = dynamic_cast<const BaseTransition*> (triggering_event());
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
		struct STransition : public Transition, sc::simple_state <STransition, TrafficLightStateMachine>
		{
			typedef  mpl::list <
				sc::transition<NormalTransition, NSTransition>,
				sc::custom_reaction <JumpTransition> > reactions;

			sc::result react(const JumpTransition& jumpTransition)
			{
				// TO FIND A WAY TO REMOVE DUBLICATE CODE
				if (jumpTransition.nextTransition_ == "EW") { return transit<EWTransition>(); }
				if (jumpTransition.nextTransition_ == "N") { return transit<NTransition>(); }
				if (jumpTransition.nextTransition_ == "S") { return transit<STransition>(); }
				if (jumpTransition.nextTransition_ == "NS") { return transit<NSTransition>(); }
				if (jumpTransition.nextTransition_ == "E") { return transit<ETransition>(); }
				if (jumpTransition.nextTransition_ == "W") { return transit<WTransition>(); }
				throw std::runtime_error("Tried to jump to undefined transition");
			}

			STransition()
			{
				auto transitionBase = dynamic_cast<const BaseTransition*> (triggering_event());
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
		struct NSTransition : public Transition, sc::simple_state <NSTransition, TrafficLightStateMachine>
		{
			typedef  mpl::list <
				sc::transition<NormalTransition, ETransition>,
				sc::custom_reaction <JumpTransition> > reactions;

			sc::result react(const JumpTransition& jumpTransition)
			{
				// TO FIND A WAY TO REMOVE DUBLICATE CODE
				if (jumpTransition.nextTransition_ == "EW") { return transit<EWTransition>(); }
				if (jumpTransition.nextTransition_ == "N") { return transit<NTransition>(); }
				if (jumpTransition.nextTransition_ == "S") { return transit<STransition>(); }
				if (jumpTransition.nextTransition_ == "NS") { return transit<NSTransition>(); }
				if (jumpTransition.nextTransition_ == "E") { return transit<ETransition>(); }
				if (jumpTransition.nextTransition_ == "W") { return transit<WTransition>(); }
				throw std::runtime_error("Tried to jump to undefined transition");
			}

			// TO BETTER DO THIS ALL OF THIS IN BASE STATE, JUST SET STRING IN HERE
			NSTransition()
			{
				auto transitionBase = dynamic_cast<const BaseTransition*> (triggering_event());
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
		struct ETransition : public Transition, sc::simple_state <ETransition, TrafficLightStateMachine>
		{
			typedef  mpl::list <
				sc::transition<NormalTransition, WTransition>,
				sc::custom_reaction <JumpTransition> > reactions;

			sc::result react(const JumpTransition& jumpTransition)
			{
				// TO FIND A WAY TO REMOVE DUBLICATE CODE
				if (jumpTransition.nextTransition_ == "EW") { return transit<EWTransition>(); }
				if (jumpTransition.nextTransition_ == "N") { return transit<NTransition>(); }
				if (jumpTransition.nextTransition_ == "S") { return transit<STransition>(); }
				if (jumpTransition.nextTransition_ == "NS") { return transit<NSTransition>(); }
				if (jumpTransition.nextTransition_ == "E") { return transit<ETransition>(); }
				if (jumpTransition.nextTransition_ == "W") { return transit<WTransition>(); }
				throw std::runtime_error("Tried to jump to undefined transition");
			}

			ETransition()
			{
				auto transitionBase = dynamic_cast<const BaseTransition*> (triggering_event());
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
		struct WTransition : public Transition, sc::simple_state <WTransition, TrafficLightStateMachine>
		{
			typedef  mpl::list <
				sc::transition<NormalTransition, EWTransition>,
				sc::custom_reaction <JumpTransition> > reactions;

			sc::result react(const JumpTransition& jumpTransition)
			{
				// TO FIND A WAY TO REMOVE DUBLICATE CODE
				if (jumpTransition.nextTransition_ == "EW") { return transit<EWTransition>(); }
				if (jumpTransition.nextTransition_ == "N") { return transit<NTransition>(); }
				if (jumpTransition.nextTransition_ == "S") { return transit<STransition>(); }
				if (jumpTransition.nextTransition_ == "NS") { return transit<NSTransition>(); }
				if (jumpTransition.nextTransition_ == "E") { return transit<ETransition>(); }
				if (jumpTransition.nextTransition_ == "W") { return transit<WTransition>(); }
				throw std::runtime_error("Tried to jump to undefined transition");
			}

			WTransition()
			{
				auto transitionBase = dynamic_cast<const BaseTransition*> (triggering_event());
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
