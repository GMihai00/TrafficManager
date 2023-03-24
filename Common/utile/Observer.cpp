#include "Observer.hpp"
#include <iostream>
namespace common
{
	namespace utile
	{
		Observer::Observer(const std::function<void()>& callback) : callback_(callback) {}

		void Observer::notify()
		{
			if (callback_)
				callback_();
		}

	}
}