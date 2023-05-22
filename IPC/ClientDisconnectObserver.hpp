#pragma once

#include <functional>
#include <mutex>
#include "net/Connection.hpp"
#include "IClientDisconnectObserver.hpp"

namespace ipc
{
	namespace utile
	{
		template<typename T>
		class ClientDisconnectObserver : public IClientDisconnectObserver<T>
		{
			const std::function<void(std::shared_ptr<ipc::net::IConnection<T>>)>& callback_;
		public:
			ClientDisconnectObserver() = delete;
			ClientDisconnectObserver(const std::function<void(std::shared_ptr<ipc::net::IConnection<T>>)>& callback)
				: callback_(callback) {}
			~ClientDisconnectObserver() = default;
			virtual void notify(std::shared_ptr<ipc::net::IConnection<T>> client) override
			{
				if (callback_)
					callback_(client);
			}
		};
	} // namespace utile
} // namespace ipc