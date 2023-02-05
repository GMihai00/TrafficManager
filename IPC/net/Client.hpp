#ifndef IPC_NET_CLIENT_HPP
#define IPC_NET_CLIENT_HPP

#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include "Message.hpp"
#include "utile/ThreadSafePriorityQueue.hpp"
#include "Connection.hpp"
#include "utile/Logger.hpp"
#include "../utile/IPCDataTypes.hpp";

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
            std::unique_ptr<Connection<T>> connection_;
            LOGGER("CLIENT");
        public:
            Client()
            {
            }

            virtual ~Client()
            {
                disconnect();
            }
    
            bool connect(const std::string& host, const ipc::utile::PORT port)
            {
                try
                {
                    boost::asio::ip::tcp::resolver resolver(context_);
                    boost::asio::ip::tcp::resolver::results_type endpoints =
                        resolver.resolve(host, std::to_string(port));

                    connection_ = std::make_unique<Connection<T>>(
                        Owner::Client,
                        context_,
                        boost::asio::ip::tcp::socket(context_),
                        incomingMessages_);
            
                    connection_->connectToServer(endpoints);
            
                    threadContext_ = std::thread([this]() { context_.run(); });
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
    
            common::utile::ThreadSafePriorityQueue<OwnedMessage<T>>& getIncomingMessages()
            {
                return incomingMessages_;
            }
    
            void send(const Message<T>& msg)
	        {
		        if (isConnected())
			        connection_->send(msg);
	        }
	
	        uint32_t getId()
	        {
                return connection_->getId();
	        }
        };
    } // namespace net
} // namespace ipc
#endif // #IPC_NET_CLIENT_HPP
