#pragma once
#ifndef COMMON_UTILE_TIMER_HPP
#define COMMON_UTILE_TIMER_HPP

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <ctime>
#include <memory>

#include "IObserver.hpp"
#include "Logger.hpp"

namespace common
{
	namespace utile
	{
		// TODO
		class Timer
		{
		protected:
			std::atomic<uint16_t> timeLeft_ = 2;
			std::time_t expirationTime_;
			std::atomic<bool> expired_ = false;
			std::atomic<bool> frozen_ = true;
			std::atomic<bool> destroy_ = false;
			std::thread threadTimer_;
			std::mutex mutexTimer_;
			std::condition_variable condVarTimer_;
			IObserverPtr observer_;
			LOGGER("TIMER");

			void startTimer();
		public:
			Timer();
			Timer(const uint16_t& sec);
			Timer(const Timer&) = delete;
			Timer(const Timer&&) = delete;
			virtual ~Timer() noexcept;
			void decreaseTimer(const uint16_t& sec);
			void resetTimer(const uint16_t& sec = 2);
			bool hasExpired();
			std::time_t getExpirationTime();
			void freezeTimer();
			void unfreezeTimer(); // NOT USED FOR ANITHING RIGHT NOW
			void subscribe(const IObserverPtr obs);
			void unsubscribe();
		};
		typedef std::shared_ptr<Timer> TimerPtr;
	} // namespace utile
} //  namespace common
#endif // #COMMON_UTILE_TIMER_HPP