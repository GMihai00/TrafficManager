#pragma once
#ifndef COMMON_UTILE_IOBSERVER_HPP
#define COMMON_UTILE_IOBSERVER_HPP

#include <memory>

namespace common
{
	namespace utile
	{
		class IObserver
		{
		public:
			virtual void notify() = 0;
		};
		typedef std::shared_ptr<IObserver> IObserverPtr;
	}
}
#endif // #COMMON_UTILE_IOBSERVER_HPP