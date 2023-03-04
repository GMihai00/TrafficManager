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

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include "Message.hpp"
#include "utile/ThreadSafePriorityQueue.hpp"
#include "Connection.hpp"
#include "utile/Logger.hpp"
#include "../utile/IPCDataTypes.hpp"
#include "../utile/IPAdressHelpers.hpp"

// REMOVE ASYNC OPERATIONS

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
            std::condition_variable condVarUpdate_;
            std::unique_ptr<Connection<T>> connection_;
            std::atomic<bool> shuttingDown_ = false;
            LOGGER("CLIENT");
        public:
            Client()
            {
            }

            virtual ~Client() noexcept
            {
                shuttingDown_ = true;
                LOG_INF << "Server shutting down";
                disconnect();
            }
    
            bool connect(const utile::IP_ADRESS& host, const ipc::utile::PORT port)
            {
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
                        condVarUpdate_);
            
                    if (connection_->connectToServer(endpoints))
                    {
                        threadContext_ = std::thread([this]() { context_.run(); });
                        return true;
                    }
           
                }
                catch(const std::exception& e)
                {
                    LOG_ERR << "Client exception: " << e.what() << '\n';
                    return false;
                }
        
                return false;
            }
    
            void disconnect()
            {
                if (isConnected())
                {
                    connection_->disconnect();
                }
        
                context_.stop();
        
                condVarUpdate_.notify_one();
                if (threadContext_.joinable())
                    threadContext_.join();
                
                connection_.release();
            }
    
            bool isConnected()
            {
                if (connection_)
                {
                    return connection_->isConnected();
                }
        
                return false;
            }
    
            bool answearRecieved()
            {
                return !incomingMessages_.empty();
            }

            std::optional<std::pair<OwnedMessage<T>, bool>> getLastUnreadAnswear()
            {
                return incomingMessages_.pop();
            }

            common::utile::ThreadSafePriorityQueue<OwnedMessage<T>>& getIncomingMessages()
            {
                return incomingMessages_;
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
                    condVarUpdate_.wait(ulock, [&] { !incomingMessages_.empty() || shuttingDown_; });

                    return !shuttingDown_;
                }

                std::unique_lock<std::mutex> ulock(mutexUpdate_);

                if (condVarUpdate_.wait_for(ulock, std::chrono::milliseconds(timeout * 100), [&] { !incomingMessages_.empty() || shuttingDown_; }))
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
                return connection_->getId();
	        }
        };
    } // namespace net
} // namespace ipc
#endif // #IPC_NET_CLIENT_HPP
