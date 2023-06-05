#include "Timer.hpp"

namespace common
{
	namespace utile
	{
		void Timer::startTimer()
		{
			threadTimer_ = std::thread([this]() {
				while (!destroy_)
				{
					std::unique_lock<std::mutex> ulock(mutexTimer_);
					condVarTimer_.wait(ulock, [this]() { return ((expired_ == false) && (frozen_ == false)) || destroy_; });
					ulock.unlock();
					while (!destroy_ && timeLeft_ > 0)
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
					expirationTime_ = std::time(NULL);
					LOG_DBG << "Timer expired";
					if (!destroy_ && observer_)
					{
						observer_->notify();
					}
					ulock.unlock();
				}
				});
		}

		Timer::Timer()
		{
			startTimer();
		}

		Timer::Timer(const uint16_t& sec) : timeLeft_(sec)
		{
			startTimer();
			if (timeLeft_ != 0)
			{
				expired_ = false;
				frozen_ = false;
				condVarTimer_.notify_one();
			}
		}

		Timer::~Timer()
		{
			destroy_ = true;
			condVarTimer_.notify_one();
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

		uint16_t Timer::getTimeLeft() const
		{
			return timeLeft_;
		}

		void Timer::resetTimer(const uint16_t& sec)
		{
			timeLeft_ = sec;
			expired_ = false;
			condVarTimer_.notify_one();
		}

		bool Timer::hasExpired() const
		{
			return expired_;
		}

		std::time_t Timer::getExpirationTime()
		{
			return expirationTime_;
		}

		void Timer::freezeTimer()
		{
			frozen_ = true;
		}

		void Timer::unfreezeTimer()
		{
			frozen_ = false;
			condVarTimer_.notify_one();
		}

		void Timer::subscribe(const IObserverPtr obs)
		{
			std::scoped_lock lock(mutexTimer_);
			if (observer_)
				observer_.reset();
			observer_ = obs;
		}

		void Timer::unsubscribe()
		{
			std::scoped_lock lock(mutexTimer_);
			if (observer_)
				observer_.reset();
		}
	} // namespace utile
} //  namespace common