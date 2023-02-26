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
			std::map<uint8_t, std::function<void()>> actionsMap_;
			std::thread threadProcess_;
			std::mutex mutexAction_;
			LOGGER("SIGHANDLER");
		public:
			SignalHandler();
			SignalHandler(const std::map<uint8_t, std::function<void()>>& actionsMap);
			~SignalHandler() noexcept = default;
			void setAction(const uint8_t& signal, const std::function<void()>& action);
		};
	} // namespace utile
} // namespace common
#endif // #COMMON_UTILE_SIGNALHANDLER_HPP
