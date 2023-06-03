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
#include "..\ClientDisconnectObserver.hpp"

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
            std::mutex mutexMessage_;
            boost::asio::ip::tcp::acceptor connectionAccepter_;
            common::utile::ThreadSafeQueue<uint32_t> availableIds_;
            std::map<uint32_t, std::shared_ptr<Connection<T>>> connections_;
            std::atomic<bool> shuttingDown_ = false;

            std::unique_ptr<ipc::utile::IClientDisconnectObserver<T>> observerDisconnect_;
            std::function<void(std::shared_ptr<ipc::net::IConnection<T>>)> disconnectCallback_;

            LOGGER("SERVER");
        public:
            void disconnectCallback(std::shared_ptr<ipc::net::IConnection<T>> connection)
            {
                std::shared_ptr<ipc::net::Connection<T>> connectionPtr =
                    std::dynamic_pointer_cast<ipc::net::Connection<T>>(connection);
                if (connectionPtr)
                {
                    onClientDisconnect(connectionPtr);
                }
            }

            Server(const utile::IP_ADRESS& host, ipc::utile::PORT port):
                connectionAccepter_(context_)
            {

                disconnectCallback_ = std::bind(&Server::disconnectCallback, this, std::placeholders::_1);

                observerDisconnect_ = std::make_unique<ipc::utile::ClientDisconnectObserver<T>>(disconnectCallback_);

                if (!utile::IsIPV4(host))
                {
                    throw std::runtime_error("Invalid IPV4 ip adress: " + host);
                }

                // throws boost::system::system_error
                boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);
                connectionAccepter_.open(endpoint.protocol());
                //connectionAccepter_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(false));
                //connectionAccepter_.set_option(boost::asio::ip::tcp::acceptor::broadcast(false));
                connectionAccepter_.bind(endpoint);
                connectionAccepter_.listen();

                // 2 much memory usage reduced to lower number
                for (uint32_t id = 0; id < 1000; id++) { availableIds_.push(id); }   
                threadUpdate_ = std::thread([&]() { LOG_INF << "START UPDATING"; while (!shuttingDown_) { update(); }});
            }

            virtual ~Server() noexcept
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
                LOG_DBG << "UPDATING: Waiting for incoming message";
                if (incomingMessagesQueue_.empty() && !shuttingDown_)
                    condVarUpdate_.wait(ulock, [&] { return !incomingMessagesQueue_.empty() || shuttingDown_; });

                if (shuttingDown_)
                {
                    return;
                }

                LOG_DBG << "UPDATING: Handling new message";
                auto maybeMsg = incomingMessagesQueue_.pop();
            
                if (maybeMsg.has_value())
                {
                    auto msg = maybeMsg.value().first;

                    if (msg.remote)
                    {
                        if (msg.msg.header.disconnecting == false)
                        {
                            onMessage(msg.remote, msg.msg);
                        }
                        else
                        {
                            messageClient(msg.remote, msg.msg);

                            msg.remote->disconnect();
                        }
                    }
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
                                    condVarUpdate_,
                                    observerDisconnect_);
                    
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
                std::scoped_lock lock(mutexMessage_);
                if (client && client->isConnected())
                {
                    client->send(msg);
                }
                else if (client)
                {
                    onClientDisconnect(client);
                    connections_.erase(client->getId());
                    availableIds_.push(client->getId());
                    client.reset();
                }
                else
                {
                    LOG_ERR << "Invalid client disconnect";
                }
            }
    
        protected:
            virtual bool onClientConnect(std::shared_ptr<Connection<T>> /*client*/)
            {
                return false;
            }
    
            virtual void onClientDisconnect(std::shared_ptr<Connection<T>> /*client*/)
            {
            }
    
            virtual void onMessage(std::shared_ptr<Connection<T>> /*client*/, Message<T>& /*msg*/)
            {
            }
        };
    } // namespace net
} // namespace ipc
#endif // #IPC_NET_SERVER_HPP
