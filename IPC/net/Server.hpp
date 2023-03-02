#pragma once
#ifndef IPC_NET_SERVER_HPP
#define IPC_NET_SERVER_HPP

#include <algorithm>
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <thread>
#include <system_error>
#include <condition_variable>

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include "utile/ThreadSafeQueue.hpp"
#include "Message.hpp"
#include "Connection.hpp"
#include "utile/Logger.hpp"
#include "../utile/IPCDataTypes.hpp"
#include "../utile/IPAdressHelpers.hpp"

namespace ipc
{
    namespace net
    {

        template<typename T>
        class Server
        {
        protected:
            common::utile::ThreadSafePriorityQueue<OwnedMessage<T>> incomingMessagesQueue_;
            boost::asio::io_context context_;
            std::thread threadContext_;
            std::thread threadUpdate_;
            std::condition_variable condVarUpdate_;
            std::mutex mutexUpdate_;
            boost::asio::ip::tcp::acceptor connectionAccepter_;
            common::utile::ThreadSafeQueue<uint32_t> availableIds_;
            std::map<uint32_t, std::shared_ptr<Connection<T>>> connections_;
            std::atomic<bool> shuttingDown_ = false;
            LOGGER("SERVER");
        public:
            Server(const utile::IP_ADRESS& host, ipc::utile::PORT port):
                connectionAccepter_(context_)
            {
                if (!utile::IsIPV4(host))
                {
                    throw std::runtime_error("Invalid IPV4 ip adress: " + host);
                }

                // throws boost::system::system_error
                boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);
                connectionAccepter_.open(endpoint.protocol());
                // THIS IS SOMEHOW FAILING
                //connectionAccepter_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(false));
                //connectionAccepter_.set_option(boost::asio::ip::tcp::acceptor::broadcast(false));
                connectionAccepter_.bind(endpoint);
                connectionAccepter_.listen();

                // 2 much memory usage reduced to lower number
                for (uint32_t id = 0; id < 1000; id++) { availableIds_.push(id); }   
                threadUpdate_ = std::thread([&]() { while (!shuttingDown_) { update(); }});
            }

            virtual ~Server()
            {
                shuttingDown_ = true;
                LOG_INF << "STOPING SERVER";
                stop();
            }

            bool start()
            {
                try
                {
                    waitForClientConnection();
                    threadContext_ = std::thread([this]() { context_.run(); });
                }
                catch(const std::exception& e)
                {
                    LOG_ERR << "Server exception";
                    logger_ << e.what() << '\n';
                    return false;
                }
        
                LOG_INF <<"Server Started\n";
                return true;
            }
    
            void stop()
            {
                LOG_DBG << "Stopping context";
                context_.stop();
                
                if (threadContext_.joinable())
                    threadContext_.join();
                
                LOG_INF << "Stopping thread update";
                condVarUpdate_.notify_one();
                if (threadUpdate_.joinable())
                    threadUpdate_.join();
    
                LOG_INF <<"Server Stopped\n";
            }
    
            void update()
            {
                std::unique_lock<std::mutex> ulock(mutexUpdate_);

                condVarUpdate_.wait(ulock, [&] { return !incomingMessagesQueue_.empty() || shuttingDown_; });

                if (shuttingDown_)
                {
                    return;
                }

                const auto& maybeMsg = incomingMessagesQueue_.pop();
            
                if (!maybeMsg.has_value())
                {
                    const auto& msg = maybeMsg.value().first;
                    onMessage(msg.remote, msg.msg);
                }
            }
            // ASYNC OK
            void waitForClientConnection()
            {
                connectionAccepter_.async_accept(
                    [this](std::error_code errcode, boost::asio::ip::tcp::socket socket)
                    {
                        if (!errcode)
                        {
                            LOG_INF << "Attempting to connect to " << socket.remote_endpoint();
                            std::shared_ptr<Connection<T>> newConnection = 
                                std::make_shared<Connection<T>>(
                                    Owner::Server, 
                                    context_,
                                    std::move(socket),
                                    incomingMessagesQueue_,
                                    condVarUpdate_);
                    
                            if (onClientConnect(newConnection))
                            {
                                auto idCounter = availableIds_.pop();
                                if (!idCounter.has_value())
                                {
                                    LOG_ERR << "Server doesn't support any more connections. Denied!";
                                    return;
                                }
                                connections_[idCounter.value()] = std::shared_ptr<Connection<T>>(std::move(newConnection));
                                connections_[idCounter.value()]->connectToClient(idCounter.value());

                                LOG_INF << connections_[idCounter.value()]->getId() << " Connection Approved";
                            }
                            else
                            {
                                LOG_WARN <<"Connection has been denied";
                            }
                        }
                        else
                        {
                            LOG_ERR << "Connection Error " << errcode.message();
                        }
                
                        waitForClientConnection();
                    });
            }
    
            void messageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg)
            {
                if (client && client->isConnected())
                {
                    client->send(msg);
                }
                else
                {
                    onClientDisconnect(client);
                    connections_.erase(client->getId());
                    availableIds_.push(client->getId());
                    client.reset();
                }
            }
    
        protected:
            virtual bool onClientConnect(std::shared_ptr<Connection<T>> client)
            {
                return false;
            }
    
            virtual void onClientDisconnect(std::shared_ptr<Connection<T>> client)
            {
            }
    
            virtual void onMessage(std::shared_ptr<Connection<T>> client,const Message<T>& msg)
            {
            }
        };
    } // namespace net
} // namespace ipc
#endif // #IPC_NET_SERVER_HPP
