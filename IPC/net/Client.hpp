#pragma once
#ifndef IPC_NET_CLIENT_HPP
#define IPC_NET_CLIENT_HPP

#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <vector>
#include <optional>
#include <condition_variable>
#include <chrono>
#include <shared_mutex>

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include "Message.hpp"
#include "utile/ThreadSafePriorityQueue.hpp"
#include "Connection.hpp"
#include "utile/Logger.hpp"
#include "../utile/IPCDataTypes.hpp"
#include "../utile/IPAdressHelpers.hpp"
#include "..\ClientDisconnectObserver.hpp"

namespace ipc
{
    namespace net
    {
        template<typename T>
        class Client
        {
        private:
            common::utile::ThreadSafePriorityQueue<OwnedMessage<T>> incomingMessages_;

        protected:
            boost::asio::io_context context_;
            std::thread threadContext_;
            std::mutex mutexUpdate_;
            std::mutex mutexGet_;
            std::shared_mutex mutexConnection_;
            std::condition_variable condVarUpdate_;
            std::unique_ptr<Connection<T>> connection_;
            std::atomic<bool> shuttingDown_ = false;
            boost::asio::io_context::work idleWork_; // for context to not immediatly stop
            std::unique_ptr<ipc::utile::IClientDisconnectObserver<T>> observer_; // needed for just creating connections, always nullptr
            LOGGER("CLIENT");
        public:
            Client() : idleWork_(context_)
            {
                threadContext_ = std::thread([this]() { context_.run(); });
            }

            virtual ~Client() noexcept
            {
                shuttingDown_ = true;
                LOG_INF << "Server shutting down";
                disconnect();
                stop();
            }
    
            bool connect(const utile::IP_ADRESS& host, const ipc::utile::PORT port)
            {
                std::unique_lock lock(mutexConnection_);

                if (!utile::IsIPV4(host))
                {
                    LOG_ERR << "Invalid IPV4 ip adress: " << host;
                    return false;
                }

                try
                {
                    boost::asio::ip::tcp::resolver resolver(context_);
                    boost::asio::ip::tcp::resolver::results_type endpoints =
                        resolver.resolve(host, std::to_string(port));

                    connection_ = std::make_unique<Connection<T>>(
                        Owner::Client,
                        context_,
                        boost::asio::ip::tcp::socket(context_),
                        incomingMessages_,
                        condVarUpdate_,
                        observer_);
            
                    return connection_->connectToServer(endpoints);

                }
                catch(const std::exception& e)
                {
                    LOG_ERR << "Client exception: " << e.what() << '\n';
                    return false;
                }
            }
    
            void disconnect()
            {
                if (isConnected())
                {
                    Message<T> disconnectMessage;
                    disconnectMessage.header.disconnecting = true;

                    send(disconnectMessage);

                    waitForAnswear(1000);

                    connection_->disconnect();

                    incomingMessages_.clear();
                }
                connection_.reset();
            }
    
            void stop()
            {
                context_.stop();

                condVarUpdate_.notify_one();
                if (threadContext_.joinable())
                    threadContext_.join();
            }

            bool isConnected()
            {
                std::shared_lock lock(mutexConnection_);

                return connection_ && connection_->isConnected();
            }
    
            bool answearRecieved()
            {
                std::shared_lock lock(mutexConnection_);

                return !incomingMessages_.empty();
            }

            std::optional<std::pair<OwnedMessage<T>, bool>> getLastUnreadAnswear()
            {
                std::scoped_lock lock(mutexGet_);

                return incomingMessages_.pop();
            }
    
            void send(const Message<T>& msg)
	        {
		        if (isConnected())
			        connection_->send(msg);
	        }

            bool waitForAnswear(uint32_t timeout = 0)
            {
                if (timeout == 0)
                {
                    std::unique_lock<std::mutex> ulock(mutexUpdate_);
                    if (incomingMessages_.empty() && !shuttingDown_)
                        condVarUpdate_.wait(ulock, [&] { return !incomingMessages_.empty() || shuttingDown_; });

                    return !shuttingDown_;
                }

                std::unique_lock<std::mutex> ulock(mutexUpdate_);
                if (!incomingMessages_.empty() || shuttingDown_)
                {
                    return !shuttingDown_;
                }

                if (condVarUpdate_.wait_for(ulock, std::chrono::milliseconds(timeout), [&] { return !incomingMessages_.empty() || shuttingDown_; }))
                {
                    return !shuttingDown_;
                }
                else
                {
                    LOG_ERR << " Answear waiting timedout";
                    return false;
                }
            }
	
	        uint32_t getId()
	        {
                std::shared_lock lock(mutexConnection_);

                return connection_->getId();
	        }
        };
    } // namespace net
} // namespace ipc
#endif // #IPC_NET_CLIENT_HPP
