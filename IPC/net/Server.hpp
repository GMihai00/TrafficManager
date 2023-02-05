#ifndef IPC_NET_SERVER_HPP
#define IPC_NET_SERVER_HPP

#include <algorithm>
#include <memory>
#include <vector>
#include <map>
#include <thread>
#include <system_error>

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include "utile/ThreadSafeQueue.hpp"
#include "Message.hpp"
#include "Connection.hpp"
#include "utile/Logger.hpp"
#include "../utile/IPCDataTypes.hpp";

namespace ipc
{
    namespace net
    {

        template<typename T>
        class Server
        {
        protected:
            common::utile::ThreadSafeQueue<OwnedMessage<T>> incomingMessagesQueue_;
            boost::asio::io_context context_;
            std::thread threadContext_;
            std::thread threadProcess_;
            boost::asio::ip::tcp::acceptor connectionAccepter_;
            std::map<uint32_t, std::shared_ptr<Connection<T>>> connections_;
            std::mutex mutexConnections_;
            uint32_t idCounter_ = 0;
            LOGGER("SERVER");
        public:
            Server(ipc::utile::PORT port):
                connectionAccepter_(context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), port))
            {
            }

            virtual ~Server()
            {
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
                context_.stop();
        
                if (threadContext_.joinable())
                    threadContext_.join();
        
                if (threadProcess_.joinable())
                    threadProcess_.join();
    
                LOG_INF <<"Server Stopped\n";
            }
    
            // THIS TO BE CHANGED TO PROCESS MESSAGES ONLY IF THERE ARE ONES AVAILABLE.
            // While true update not the best solution...
            void update(size_t maxMessages = -1)
            {
                size_t messageCount = 0;
                while (messageCount < maxMessages && !incomingMessagesQueue_.empty())
                {
                    const auto& msg = incomingMessagesQueue_.pop().first;
            
                    onMessage(msg.remote, msg.msg);
            
                    messageCount++;
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
                            LOG_INF << "Connection succeded " << socket.remote_endpoint();
                            std::shared_ptr<Connection<T>> newConnection = 
                                std::make_shared<Connection<T>>(
                                    Owner::Server, 
                                    context_,
                                    std::move(socket),
                                    incomingMessagesQueue_);
                    
                            if (onClientConnect(newConnection))
                            {
                                mutexConnections_.lock();
                                uint32_t looped = 0;
                                while (connections_.find(idCounter_) != connections_.end())
                                {
                                    idCounter_++;
                                    looped++;
                                    if (looped == 0)
                                    {
                                        LOG_ERR << "Server doesn't support any more connections. Denied!";
                                        mutexConnections_.unlock();
                                        return;
                                    }
                                }
                                connections_[idCounter_] = std::shared_ptr<Connection>(std::move(newConnection));
                                connections_[idCounter_]->connectToClient(idCounter_);
                                mutexConnections_.unlock();

                                LOG_INF << connections_[idCounter_]->getId() << " Connection Approved";
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
                    client.reset();
                    connections_.erase(client->getId());
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
