#pragma once
#ifndef IPC_NET_CONNECTION_HPP
#define IPC_NET_CONNECTION_HPP

#include <iostream>
#include <memory>
#include <vector>
#include <system_error>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include "..\IClientDisconnectObserver.hpp"

#include "Message.hpp"
#include "utile/ThreadSafePriorityQueue.hpp"
#include "utile/Logger.hpp"
#include "..\IConnection.hpp"

namespace ipc
{
    namespace net
    {
        enum class Owner
        {
            Server,
            Client
        };

        template <typename T>
        class Connection : public IConnection<T>, public std::enable_shared_from_this<Connection<T>>
        {
        protected:
            const Owner owner_;
            std::thread threadRead_;
            std::thread threadWrite_;
            std::mutex mutexRead_;
            std::mutex mutexWrite_;
            std::condition_variable condVarRead_;
            std::condition_variable condVarWrite_;
            std::condition_variable& condVarUpdate_;
            boost::asio::io_context& context_;
            boost::asio::ip::tcp::socket socket_;
            common::utile::ThreadSafePriorityQueue<OwnedMessage<T>>& incomingMessages_;
            common::utile::ThreadSafePriorityQueue<Message<T>> outgoingMessages_;
            Message<T> incomingTemporaryMessage_;
            std::atomic_bool isReading_;
            std::atomic_bool isWriting_;
            std::atomic_bool shuttingDown_ = false;
            uint32_t id_;
            std::string ipAdress_;

            std::unique_ptr<ipc::utile::IClientDisconnectObserver<T>>& observer_;
            LOGGER("CONNECTION-UNDEFINED");

        private:
            struct compareConnections {
                bool operator() (const Connection<T>& a,const Connection<T>& b) const {
                    return a.getId() < b.getId();
                }
            };

            bool readData(std::vector<uint8_t>& vBuffer, size_t toRead)
            {
                size_t left = toRead;
                while (left && !shuttingDown_)
                {
                    if (shuttingDown_)
                        return false;
            
                    boost::system::error_code errcode;
                    size_t read = socket_.read_some(boost::asio::buffer(vBuffer.data() + (toRead - left), left), errcode);
            
                    if (errcode)
                    {
                        
                        LOG_ERR << "Error while reading data err: " << errcode.value() << errcode.message();
                        disconnect();
                        return false;
                    }
                    left -= read;
                }
                return true;
            }
        public:
            Connection(Owner owner,
                boost::asio::io_context& context,
                boost::asio::ip::tcp::socket socket,
                common::utile::ThreadSafePriorityQueue<OwnedMessage<T>>& incomingMessages,
                std::condition_variable& condVarUpdate,
                std::unique_ptr<ipc::utile::IClientDisconnectObserver<T>>& observer) :
                owner_{ owner },
                context_{ context },
                socket_{ std::move(socket) },
                incomingMessages_{ incomingMessages },
                condVarUpdate_{condVarUpdate},
                observer_{observer},
                isWriting_{false},
                isReading_{false},
                id_{0}
            {
                threadWrite_ = std::thread([this]() { writeMessages(); });
                threadRead_ = std::thread([this]() { readMessages(); });
            }

            virtual ~Connection() noexcept
            { 
                shuttingDown_ = true;

                condVarRead_.notify_one();
                if (threadRead_.joinable())
                    threadRead_.join();

                condVarWrite_.notify_one();
                if (threadWrite_.joinable())
                    threadWrite_.join();

                disconnect(); 
            }
    
            Owner getOwner() const
            {
                return owner_;
            }

            uint32_t getId() const
            {
                return id_;
            }
    
            std::string getIpAdress() const
            {
                return ipAdress_;
            }

            bool connectToServer(const boost::asio::ip::tcp::resolver::results_type& endpoints)
            {
                if (owner_ == Owner::Client)
                {
                    LOG_SET_NAME("CONNECTION-SERVER");
                    std::function<void(std::error_code errcode, boost::asio::ip::tcp::endpoint endpoint)> connectCallback;
                    if (isReading_)
                    {
                        connectCallback = [this](std::error_code errcode, boost::asio::ip::tcp::endpoint /*endpoint*/)
                        {
                            if (errcode)
                            {
                                LOG_ERR << "FAILED TO CONNECT TO SERVER: " << errcode.message();
                                socket_.close();
                            }
                        };
                    }
                    else
                    {
                        connectCallback = [this](std::error_code errcode, boost::asio::ip::tcp::endpoint /*endpoint*/)
                        {
                            if (!errcode)
                            {
                                LOG_DBG <<"Started reading messages:";
                                condVarRead_.notify_one();
                            }
                            else
                            {
                                LOG_ERR << "FAILED TO CONNECT TO SERVER: " << errcode.message();
                                socket_.close();
                            }
                        };
                    }

                    std::error_code ec;
                    boost::asio::ip::tcp::endpoint endpoint;
                    try
                    {
                        endpoint = boost::asio::connect(socket_, endpoints);
                    }
                    catch (boost::system::system_error const& err)
                    {
                        ec = err.code();
                    }
                   
                    connectCallback(ec, endpoint);
                    if (socket_.is_open())
                    {
                        isReading_ = true;
                        ipAdress_ = socket_.remote_endpoint().address().to_string();
                        return true;
                    }

                    return false;
                }
                return false;
            }
    
            bool connectToClient(uint32_t id)
            {
                if (owner_ == Owner::Server)
                {
                    if (socket_.is_open())
                    {
                        id_ = id;
                        LOG_SET_NAME("CONNECTION-" + std::to_string(id_));
                        std::function<void()> connectCallback;
                        if (!isReading_)
                        {
                            connectCallback = [this]()
                            {
                                LOG_DBG <<"Started reading messages";
                                condVarRead_.notify_one();
                            };
                        }
                        else
                        {
                            connectCallback =[this]()
                            {
                            };
                        }
                        isReading_ = true;
                        connectCallback();
                        return true;
                    }
                    return false;
                }
                return false;
            }
    
            bool isConnected() const
            {
                return socket_.is_open();
            }
    
            std::shared_ptr<Connection<T>> get_shared()
            {
                if (shuttingDown_)
                    return nullptr;

                return this->shared_from_this();
            }
            void readMessages()
            {
                if (!isReading_ && !shuttingDown_)
                {
                    std::unique_lock<std::mutex> ulock(mutexRead_);
                    condVarRead_.wait(ulock, [this]() { return isReading_ || shuttingDown_; });
                    ulock.unlock();
                }

                while (!shuttingDown_)
                {
                    std::scoped_lock lock(mutexRead_);
                    if (!readHeader()) { break; }
                    if (!readBody()) { break; }
                    addToIncomingMessageQueue();
                    condVarUpdate_.notify_one();
                }
                isReading_ = false;
            }

            bool readHeader()
            {
                std::vector<uint8_t> vBuffer(sizeof(MessageHeader<T>));
        
                if (!readData(vBuffer, sizeof(MessageHeader<T>))) { return false; }

                std::memcpy(&incomingTemporaryMessage_.header, vBuffer.data(), sizeof(MessageHeader<T>));
                LOG_DBG << "Finished reading header for message: " << incomingTemporaryMessage_;
                return true;
            }
    
            bool readBody()
            {
                std::vector<uint8_t> vBuffer(incomingTemporaryMessage_.header.size * sizeof(uint8_t));
    
                if (!readData(vBuffer, sizeof(uint8_t) * incomingTemporaryMessage_.header.size)) { return false; }

                incomingTemporaryMessage_ << vBuffer;
                LOG_DBG << "Finished reading message: " << incomingTemporaryMessage_;
                return true;
            }
    
            void addToIncomingMessageQueue()
            {
                if (owner_ == Owner::Server)
                {
                    const auto& pair = std::make_pair(
                        OwnedMessage<T>{get_shared(), incomingTemporaryMessage_},
                        incomingTemporaryMessage_.header.hasPriority);
                    incomingMessages_.push(pair);
                }
                else
                {
                    incomingMessages_.push(
                        std::make_pair(
                            OwnedMessage<T>{nullptr, incomingTemporaryMessage_},
                            incomingTemporaryMessage_.header.hasPriority
                        ));
                }
                incomingTemporaryMessage_.clear();
                LOG_DBG << "Added message to incoming queue";
            }

            void send(const Message<T>& msg)
            {
                std::function<void()> postCallback;
                std::pair<Message<T>, bool> pair = std::make_pair(msg, msg.header.hasPriority);
                outgoingMessages_.push(pair);
                LOG_DBG <<"Adding message to outgoing queue: " << msg;
                if (isWriting_)
                {
                    postCallback = [this, msg]()
                    {
                    };
                }
                else
                {
                    postCallback = [this, msg]()
                    {
                        LOG_DBG <<"Started writing messages";
                        condVarWrite_.notify_one();
                    };
                }
                isWriting_ = true;

                if (isConnected())
                {
                    boost::asio::post(context_, postCallback);
                }
                else
                {
                    LOG_WARN << "Failed to post message, client is disconnected";
                }
            }
    
            void writeMessages()
            {
                while (!shuttingDown_)
                {
                    if (!isWriting_ && !shuttingDown_)
                    {
                        std::unique_lock<std::mutex> ulock(mutexWrite_);
                        condVarWrite_.wait(ulock, [this]() { return isWriting_ || shuttingDown_; });
                        ulock.unlock();
                    }

                    while (!outgoingMessages_.empty())
                    {
                        std::scoped_lock lock(mutexWrite_);

                        if (shuttingDown_)
                            break;

                        auto outgoingMsg = outgoingMessages_.pop();
                        if (!outgoingMsg)
                        {
                            LOG_ERR << "Failed to get image from queue";
                            return;
                        } 
                        const auto& outgoingMessage = outgoingMsg.value().first;

                        LOG_DBG << "Started writing message: " << outgoingMessage;
                        if (!writeHeader(outgoingMessage)) { outgoingMessages_.clear(); break; }

                        if (outgoingMessage.header.size > 0)
                        {
                            if (!writeBody(outgoingMessage)) { outgoingMessages_.clear(); break; }
                        }
                        else
                        {
                            LOG_DBG << "Finished writing message ";
                        }
                    }
                    isWriting_ = false;
                }
            }
    
            bool writeHeader(const Message<T>& outgoingMessage)
            {
                boost::system::error_code errcode;
                boost::asio::write(socket_, boost::asio::buffer(&outgoingMessage.header, sizeof(MessageHeader<T>)), errcode);
        
                if (errcode)
                {
                    LOG_ERR << "Failed to write message header: " << errcode.message();
                    disconnect();
                    return false;
                }
                LOG_DBG << "Finished writing header";
                return true;
            }
    
            bool writeBody(const Message<T>& outgoingMessage)
            {
                boost::system::error_code errcode;
                boost::asio::write(socket_, boost::asio::buffer(outgoingMessage.body.data(), sizeof(uint8_t) * outgoingMessage.size()), errcode);
        
                if (errcode)
                {
                    LOG_ERR << "Failed to write message body: " << errcode.message();
                    disconnect();
                    return false;
                }
                LOG_DBG << "Finished writing message";
                return true;
            }
    
            void disconnect()
            {
                if (isConnected())
                {
                    if (observer_)
                        observer_->notify(this->shared_from_this());

                    boost::asio::post(context_, [this]() { socket_.close(); });
                }
            }
        };
    }   // namespace net
}   // namespace ipc
#endif // #IPC_NET_CONNECTION_HPP
