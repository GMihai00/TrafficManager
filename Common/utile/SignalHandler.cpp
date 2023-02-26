#include "SignalHandler.hpp"

namespace common
{
	namespace utile
	{
		// TO FINALIZE THIS NEED TO DEFINE THREAD THAT DETECTS NEW SIGNALS
		SignalHandler::SignalHandler()
		{

		}
		SignalHandler::SignalHandler(const std::map<uint8_t, std::function<void()>>& actionsMap) :
			actionsMap_(actionsMap)
		{

		}

		void SignalHandler::setAction(const uint8_t& signal, const std::function<void()>& action)
		{
			std::scoped_lock lock(mutexAction_);
			actionsMap_[signal] = action;
		}

	} // namespace utile
} // namespace common