#pragma once
#ifndef COMMON_UTILE_OBSERVER_HPP
#define COMMON_UTILE_OBSERVER_HPP

#include <functional>
#include <mutex>
#include "IObserver.hpp"

namespace common
{
	namespace utile
	{
		class Observer : public IObserver
		{
			std::mutex mutexNotify_;
			const std::function<void()>& callback_;
		public:
			Observer() = delete;
			Observer(const std::function<void()>& callback);
			~Observer() = default;
			virtual void notify() override;
		};
	} // namespace utile
} // namespace common

#endif // #COMMON_UTILE_OBSERVER_HPP