#pragma once

#include "IConnection.hpp"
namespace ipc
{
	namespace utile
	{
		template<typename T>
		class IClientDisconnectObserver
		{
		public:
			virtual void notify(std::shared_ptr<ipc::net::IConnection<T>> client) {};
		};
	} // namespace utile
} // namespace ipc