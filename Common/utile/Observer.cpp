#include "Observer.hpp"

namespace common
{
	namespace utile
	{
		Observer::Observer(const std::function<void()>& callback) : callback_(callback) {}

		void Observer::notify()
		{
			std::scoped_lock lock(mutexNotify_);
			callback_();
		}

		bool Observer::operator== (const IObserver& rhs)
		{

		}

	}
}