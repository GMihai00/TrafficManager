#include "SignalHandler.hpp"

namespace common
{
	namespace utile
	{
		void SignalHandler::setAction(const uint8_t& sig, const _crt_signal_t& action)
		{
			std::scoped_lock lock(mutexAction_);
			signal(sig, action);
		}

	} // namespace utile
} // namespace common