#include "Timer.hpp"

namespace common
{
	namespace utile
	{
		Timer::Timer(const uint16_t& sec) : timeLeft_(sec)
		{
			if (timeLeft_ != 0)
			{
				expired_ = false;
				frozen_ = false;
			}
			threadTimer_ = std::thread([this]() {
					while (true)
					{
						std::unique_lock<std::mutex> ulock(mutexTimer_);
						condVarTimer_.wait(ulock, [this]() { return (expired_ == false) && (frozen_ == false); });
						ulock.unlock();
						while (timeLeft_ > 0)
						{
							ulock.lock();
							std::this_thread::sleep_for(std::chrono::seconds(1));
							timeLeft_--;
							if (frozen_)
							{
								condVarTimer_.wait(ulock, [this]() { return (frozen_ == false); });
							}
							ulock.unlock();
						}
						ulock.lock();
						expired_ = true;
						if (observer_)
						{
							observer_->notify();
						}
						ulock.unlock();
					}
				});
			condVarTimer_.notify_one();
		}

		Timer::~Timer()
		{
			if (threadTimer_.joinable())
				threadTimer_.join();
		}

		void Timer::decreaseTimer(const uint16_t& sec)
		{
			std::scoped_lock lock(mutexTimer_);
			if (timeLeft_ < sec)
				timeLeft_ = 0;
			else
				timeLeft_ -= sec;
		}

		bool Timer::resetTimer(const uint16_t& sec = 60)
		{
			std::scoped_lock lock(mutexTimer_);
			timeLeft_ = sec;
			expired_ = true;
			condVarTimer_.notify_one();
		}

		bool Timer::hasExpired(const uint16_t& sec)
		{
			std::scoped_lock lock(mutexTimer_);
			return expired_;
		}

		void Timer::freezeTimer()
		{
			std::scoped_lock lock(mutexTimer_);
			frozen_ = true;
		}

		void Timer::unfreezeTimer()
		{
			std::scoped_lock lock(mutexTimer_);
			frozen_ = false;
			condVarTimer_.notify_one();
		}

		bool Timer::subscribe(const IObserverPtr obs)
		{
			std::scoped_lock lock(mutexTimer_);
			if (observer_)
				observer_.reset();
			observer_ = obs;
		}

		bool Timer::unsubscribe()
		{
			if (observer_)
				observer_.reset();
		}
	} // namespace utile
} //  namespace common