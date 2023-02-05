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

		// EVENTS
		struct Start : sc::event<Start>
		{
		};

		struct NormalTransition : sc::event<NormalTransition>
		{
		};

		struct JumpTransition : sc::event<JumpTransition>
		{
		};

		struct Transition
		{
			static std::shared_ptr<Transition> nextTransition_;
			virtual sc::result react(const JumpTransition& ev) = 0;
			virtual ~Transition() noexcept { nextTransition_.reset(); };
		};

		struct Stopped;
		struct EVTransition;
		struct NTransition;
		struct STransition;
		struct NSTransition;
		struct ETransition;
		struct WTransition;
		struct TrafficLightStateMachine : sc::state_machine<TrafficLightStateMachine, Stopped>
		{
		private:
			// CONFIG DATA
			bool usingLeftLane_;
			const uint16_t maximumWaitingTime_; // THIS IS USED FOR TIMERS 
			boost::optional<common::utile::LANE> missingLane_;

			std::mutex mutexClients_;
			std::map<common::utile::LANE, ipc::utile::IP_ADRESS> laneToVehicleTrackerIPAdress_;
			std::map<common::utile::LANE, ipc::utile::IP_ADRESSES> clientsConnected_;
			std::map<common::utile::LANE, common::utile::Timer> laneToTimerMap_;

			// IS TREATED AS A CLIENT SO CAN CHECK INSIDE CLIENTSCONNECTED
			common::utile::ThreadSafeQueue<std::pair<common::utile::LANE, ipc::utile::IP_ADRESS>> waitingEmergencyVehicles_;
			
			IObserverPtr greenLightObserver_;
			common::utile::Timer greenLightTimer_;
			LOGGER("TRAFFICLIGHT-STATEMACHINE");

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

			void decreaseTimer(const common::utile::LANE lane, ipc::utile::IP_ADRESS ip);
			void greenLightExpireCallback();
		};

		// STATES
		struct Stopped : sc::simple_state <Stopped, TrafficLightStateMachine>
		{
			typedef sc::transition<Start ,EVTransition> reactions;
		};

		// STATE I/IV
		struct EVTransition : public Transition, sc::simple_state <EVTransition, TrafficLightStateMachine>
		{
			typedef  mpl::list < 
				sc::transition<NormalTransition, NTransition>,
				sc::custom_reaction <JumpTransition> > reactions;

			sc::result react(const JumpTransition& ev)
			{
				// TO FIND A WAY TO REMOVE DUBLICATES
				if (!nextTransition_)
				{
					throw std::runtime_error("Tried to jump to undefined transition");
				}
				if (std::dynamic_pointer_cast<EVTransition>(nextTransition_) != nullptr) { return transit<EVTransition>(); }
				if (std::dynamic_pointer_cast<NTransition>(nextTransition_) != nullptr) { return transit<NTransition>(); }
				if (std::dynamic_pointer_cast<STransition>(nextTransition_) != nullptr) { return transit<STransition>(); }
				if (std::dynamic_pointer_cast<NSTransition>(nextTransition_) != nullptr) { return transit<NSTransition>(); }
				if (std::dynamic_pointer_cast<ETransition>(nextTransition_) != nullptr) { return transit<ETransition>(); }
				if (std::dynamic_pointer_cast<WTransition>(nextTransition_) != nullptr) { return transit<WTransition>(); }
			}
		};

		// STATE II
		struct NTransition : public Transition, sc::simple_state <NTransition, TrafficLightStateMachine>
		{
			typedef  mpl::list <
				sc::transition<NormalTransition, STransition>,
				sc::custom_reaction <JumpTransition> > reactions;

			sc::result react(const JumpTransition& ev)
			{
				if (!nextTransition_)
				{
					throw std::runtime_error("Tried to jump to undefined transition");
				}
				if (std::dynamic_pointer_cast<EVTransition>(nextTransition_) != nullptr) { return transit<EVTransition>(); }
				if (std::dynamic_pointer_cast<NTransition>(nextTransition_) != nullptr) { return transit<NTransition>(); }
				if (std::dynamic_pointer_cast<STransition>(nextTransition_) != nullptr) { return transit<STransition>(); }
				if (std::dynamic_pointer_cast<NSTransition>(nextTransition_) != nullptr) { return transit<NSTransition>(); }
				if (std::dynamic_pointer_cast<ETransition>(nextTransition_) != nullptr) { return transit<ETransition>(); }
				if (std::dynamic_pointer_cast<WTransition>(nextTransition_) != nullptr) { return transit<WTransition>(); }
			}
		};

		// STATE III
		struct STransition : public Transition, sc::simple_state <STransition, TrafficLightStateMachine>
		{
			typedef  mpl::list <
				sc::transition<NormalTransition, NSTransition>,
				sc::custom_reaction <JumpTransition> > reactions;

			sc::result react(const JumpTransition& ev)
			{
				if (!nextTransition_)
				{
					throw std::runtime_error("Tried to jump to undefined transition");
				}
				if (std::dynamic_pointer_cast<EVTransition>(nextTransition_) != nullptr) { return transit<EVTransition>(); }
				if (std::dynamic_pointer_cast<NTransition>(nextTransition_) != nullptr) { return transit<NTransition>(); }
				if (std::dynamic_pointer_cast<STransition>(nextTransition_) != nullptr) { return transit<STransition>(); }
				if (std::dynamic_pointer_cast<NSTransition>(nextTransition_) != nullptr) { return transit<NSTransition>(); }
				if (std::dynamic_pointer_cast<ETransition>(nextTransition_) != nullptr) { return transit<ETransition>(); }
				if (std::dynamic_pointer_cast<WTransition>(nextTransition_) != nullptr) { return transit<WTransition>(); }
			}
		};

		// STATE V/VIII
		struct NSTransition : public Transition, sc::simple_state <NSTransition, TrafficLightStateMachine>
		{
			typedef  mpl::list <
				sc::transition<NormalTransition, ETransition>,
				sc::custom_reaction <JumpTransition> > reactions;

			sc::result react(const JumpTransition& ev)
			{
				if (!nextTransition_)
				{
					throw std::runtime_error("Tried to jump to undefined transition");
				}
				if (std::dynamic_pointer_cast<EVTransition>(nextTransition_) != nullptr) { return transit<EVTransition>(); }
				if (std::dynamic_pointer_cast<NTransition>(nextTransition_) != nullptr) { return transit<NTransition>(); }
				if (std::dynamic_pointer_cast<STransition>(nextTransition_) != nullptr) { return transit<STransition>(); }
				if (std::dynamic_pointer_cast<NSTransition>(nextTransition_) != nullptr) { return transit<NSTransition>(); }
				if (std::dynamic_pointer_cast<ETransition>(nextTransition_) != nullptr) { return transit<ETransition>(); }
				if (std::dynamic_pointer_cast<WTransition>(nextTransition_) != nullptr) { return transit<WTransition>(); }
			}
		};

		// STATE VI
		struct ETransition : public Transition, sc::simple_state <ETransition, TrafficLightStateMachine>
		{
			typedef  mpl::list <
				sc::transition<NormalTransition, WTransition>,
				sc::custom_reaction <JumpTransition> > reactions;

			sc::result react(const JumpTransition& ev)
			{
				if (!nextTransition_)
				{
					throw std::runtime_error("Tried to jump to undefined transition");
				}
				if (std::dynamic_pointer_cast<EVTransition>(nextTransition_) != nullptr) { return transit<EVTransition>(); }
				if (std::dynamic_pointer_cast<NTransition>(nextTransition_) != nullptr) { return transit<NTransition>(); }
				if (std::dynamic_pointer_cast<STransition>(nextTransition_) != nullptr) { return transit<STransition>(); }
				if (std::dynamic_pointer_cast<NSTransition>(nextTransition_) != nullptr) { return transit<NSTransition>(); }
				if (std::dynamic_pointer_cast<ETransition>(nextTransition_) != nullptr) { return transit<ETransition>(); }
				if (std::dynamic_pointer_cast<WTransition>(nextTransition_) != nullptr) { return transit<WTransition>(); }
			}
		};

		// STATE VII
		struct WTransition : public Transition, sc::simple_state <WTransition, TrafficLightStateMachine>
		{
			typedef  mpl::list <
				sc::transition<NormalTransition, EVTransition>,
				sc::custom_reaction <JumpTransition> > reactions;

			sc::result react(const JumpTransition& ev)
			{
				if (!nextTransition_)
				{
					throw std::runtime_error("Tried to jump to undefined transition");
				}
				if (std::dynamic_pointer_cast<EVTransition>(nextTransition_) != nullptr) { return transit<EVTransition>(); }
				if (std::dynamic_pointer_cast<NTransition>(nextTransition_) != nullptr) { return transit<NTransition>(); }
				if (std::dynamic_pointer_cast<STransition>(nextTransition_) != nullptr) { return transit<STransition>(); }
				if (std::dynamic_pointer_cast<NSTransition>(nextTransition_) != nullptr) { return transit<NSTransition>(); }
				if (std::dynamic_pointer_cast<ETransition>(nextTransition_) != nullptr) { return transit<ETransition>(); }
				if (std::dynamic_pointer_cast<WTransition>(nextTransition_) != nullptr) { return transit<WTransition>(); }
			}
		};

	} // namespace controller
} // namespace model
#endif // #MODEL_CONTROLLER_TRAFFICLIGHTSTATEMACHINE_HPP
