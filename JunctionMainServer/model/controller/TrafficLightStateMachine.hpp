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
#include "utile/Timer.hpp"

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

		struct JumpTransition;
		struct Transition // JUST TO HAVE A BASE CLASS FOR ALL OF THEM
		{
			static std::shared_ptr<Transition> nextTransition_;
			virtual sc::result react(const JumpTransition& ev) = 0; // FOR NOW LIKE THIS BUT COULD IMPLEMENT IN HERE FOR ALL
			virtual ~Transition();
		};

		struct JumpTransition : sc::event<JumpTransition>
		{
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
		public:
			std::mutex mutexQueue_;
			std::map<uint8_t, uint16_t> trafficLoadMap_;
			std::map <uint8_t, common::utile::Timer> laneToTimerMap_;
			std::map<uint8_t, std::set<uint32_t>> vtsConnected_;
			std::queue<std::pair<uint32_t, uint8_t>> queuedEmergencyVehicles_;
			std::set<uint32_t> emergencyVehiclesWaiting_;
			const uint16_t maximumWaitingTime_;
			boost::optional<uint8_t> missingLane_;
			common::utile::Timer greenLightTimer_;
			const uint16_t changeAdditionalTime_ = 5;
			LOGGER("TRAFFICLIGHT-STATEMACHINE");

			TrafficLightStateMachine(const uint16_t maximumWaitingTime,const boost::optional<uint8_t>& missingRoad);
			TrafficLightStateMachine(const TrafficLightStateMachine&) = delete;
			~TrafficLightStateMachine() = default;

			bool isLaneMissing(const uint8_t lane);
			// BASED ON THE LANE WE WILL DETERMINE WHAT PHAZE TO START: CAN BE EITHER II, III, VI or VII
			// as they are the only ones that allow the vehicles to move freely from one lane
			bool startEmergencyState(const uint32_t connectionId, const uint8_t lane);
			bool isInEmergencyState();
			bool endEmergencyState(const uint32_t connectionId, const uint8_t lane);

			void registerDectedCars(const uint16_t numberOfCars, const uint8_t lane);
			bool registerVT(const uint32_t connectionId, const uint8_t lane, const bool incoming);
			static std::shared_ptr<Transition> nextTransition;
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
