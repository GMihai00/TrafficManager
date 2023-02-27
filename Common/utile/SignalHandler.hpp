#pragma once
#pragma once
#ifndef COMMON_UTILE_SIGNALHANDLER_HPP
#define COMMON_UTILE_SIGNALHANDLER_HPP

#include <csignal>
#include <thread>
#include <mutex>
#include <functional>
#include <map>

#include "Logger.hpp"

namespace common
{
	namespace utile
	{
		class SignalHandler
		{
		protected:
			std::mutex mutexAction_;
			LOGGER("SIGHANDLER");
		public:
			SignalHandler() = default;
			~SignalHandler() noexcept = default;
			void setAction(const uint8_t& sig, const _crt_signal_t& action);
		};
	} // namespace utile
} // namespace common
#endif // #COMMON_UTILE_SIGNALHANDLER_HPP
