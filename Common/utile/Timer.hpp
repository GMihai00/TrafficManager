#pragma once
#ifndef COMMON_UTILE_TIMER_HPP
#define COMMON_UTILE_TIMER_HPP

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>

#include "IObserver.hpp"

namespace common
{
	namespace utile
	{
		// TODO
		class Timer
		{
		protected:
			std::atomic<uint16_t> timeLeft_ = 0;
			std::atomic<bool> expired_ = true;
			std::atomic<bool> frozen_ = true;
			std::thread threadTimer_;
			std::mutex mutexTimer_;
			std::condition_variable condVarTimer_;
			IObserverPtr observer_;
		public:
			Timer() = delete;
			Timer(const uint16_t& sec = 0);
			Timer(const Timer&) = delete;
			virtual ~Timer() noexcept;
			void decreaseTimer(const uint16_t& sec);
			bool resetTimer(const uint16_t& sec = 60);
			bool hasExpired(const uint16_t& sec);
			void freezeTimer();
			void unfreezeTimer();
			bool subscribe(const IObserverPtr obs);
			bool unsubscribe();
		};
	} // namespace utile
} //  namespace common
#endif // #COMMON_UTILE_TIMER_HPP