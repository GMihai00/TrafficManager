#ifndef IPC_NET_CONNECTION_HPP
#define IPC_NET_CONNECTION_HPP

#include <iostream>
#include <memory>
#include <vector>
#include <system_error>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include "Message.hpp"
#include "utile/ThreadSafePriorityQueue.hpp"
#include "utile/Logger.hpp"

namespace ipc
{
namespace net
{
    enum class Owner
    {
        Server,
        Proxy,
        Client
    };

    template <typename T>
    class Connection : public std::enable_shared_from_this<Connection<T>>
    {
    protected:
        const Owner owner_;
        std::thread threadRead_;
        std::thread threadWrite_;
        std::mutex mutexStartRead_;
        std::mutex mutexRead_;
        std::mutex mutexWrite_;
        std::mutex mutexSend_;
        boost::asio::io_context& context_;
        boost::asio::ip::tcp::socket socket_;
        common::utile::ThreadSafePriorityQueue<OwnedMessage<T>>& incomingMessages_;
        common::utile::ThreadSafePriorityQueue<Message<T>> outgoingMessages_;
        Message<T> incomingTemporaryMessage_;
        bool isReading;
        bool isWriting;
        uint32_t id_;
        LOGGER("CONNECTION-UNDEFINED");

    private:
        struct compareConnections {
            bool operator() (const Connection<T>& a,const Connection<T>& b) const {
                return a.getId() < b.getId();
            }
        };
        void readData(std::vector<uint8_t>& vBuffer, size_t toRead)
        {
            size_t left = toRead;
            while (left)
            {
                size_t available = socket_.available();
                size_t canRead = std::min(left, available);
            
                boost::system::error_code errcode;
                socket_.read_some(boost::asio::buffer(vBuffer.data() + (toRead - left), canRead), errcode);
            
                if (errcode)
                {
                    LOG_ERR << "Error while reading data";
                    disconnect();
                    return;
                }
                left -= canRead;
            }
        }
    public:
        Connection(Owner owner,
            boost::asio::io_context& context,
            boost::asio::ip::tcp::socket socket,
            common::utile::ThreadSafePriorityQueue<OwnedMessage<T>>& incomingMessages) :
            owner_{ owner },
            context_{ context },
            socket_{ std::move(socket) },
            incomingMessages_{ incomingMessages },
            isWriting{false},
            isReading{false},
            id_{0}
        {
        }

        virtual ~Connection() {}
    
        uint32_t getId() const
        {
            return id_;
        }
    
        // ASYNC OK
        void connectToServer(const boost::asio::ip::tcp::resolver::results_type& endpoints)
        {
            if (owner_ == Owner::Client || owner_ == Owner::Proxy)
            {
                LOG_SET_NAME("CONNECTION-SERVER");
                std::function<void(std::error_code errcode, boost::asio::ip::tcp::endpoint endpoint)> connectCallback;
                mutexStartRead_.lock();
                if (isReading)
                {
                    connectCallback = [this](std::error_code errcode, boost::asio::ip::tcp::endpoint endpoint)
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
                    connectCallback = [this](std::error_code errcode, boost::asio::ip::tcp::endpoint endpoint)
                    {
                        if (!errcode)
                        {
                            LOG_DBG <<"Started reading messages:";
                            threadRead_ = std::thread([this](){ readMessages(); });
                        }
                        else
                        {
                            LOG_ERR << "FAILED TO CONNECT TO SERVER: " << errcode.message();
                            socket_.close();
                        }
                    };
                }
                isReading = true;
                mutexStartRead_.unlock();
                boost::asio::async_connect(socket_, endpoints, connectCallback);
            }
        }
    
        void connectToClient(uint32_t id)
        {
            if (owner_ == Owner::Server || owner_ == Owner::Proxy)
            {
                if (socket_.is_open())
                {
                    id_ = id;
                    LOG_SET_NAME("CONNECTION-" + std::to_string(id_));
                    std::function<void()> connectCallback;
                    mutexStartRead_.lock();
                    if (!isReading)
                    {
                        connectCallback = [this]()
                        {
                            LOG_DBG <<"Started reading messages";
                            threadRead_ = std::thread([this](){ readMessages(); });
                        };
                    }
                    else
                    {
                        connectCallback =[this]()
                        {
                        };
                    }
                    isReading = true;
                    mutexStartRead_.unlock();
                    connectCallback();
                }
            }
        }
    
        bool isConnected() const
        {
            return socket_.is_open();
        }
    
        void readMessages()
        {
            while (true)
            {
                mutexRead_.lock();
                readHeader();
                readBody();
                addToIncomingMessageQueue();
                mutexRead_.unlock();
            }
        }

        void readHeader()
        {
            std::vector<uint8_t> vBuffer(sizeof(MessageHeader<T>));
        
            readData(vBuffer, sizeof(MessageHeader<T>));
            LOG_DBG << "Finished reading header for message: " << incomingTemporaryMessage_;
            std::memcpy(&incomingTemporaryMessage_.header, vBuffer.data(), sizeof(MessageHeader<int>));
        }
    
        void readBody()
        {
            std::vector<uint8_t> vBuffer(incomingTemporaryMessage_.header.size);
    
            readData(vBuffer, sizeof(incomingTemporaryMessage_.header.size));
            LOG_DBG << "Finished reading message: " << incomingTemporaryMessage_;
            incomingTemporaryMessage_ << vBuffer;
        }
    
        void addToIncomingMessageQueue()
        {
            if (owner_ == Owner::Server || owner_ == Owner::Proxy)
            {
                const auto& pair = std::make_pair(
                    OwnedMessage<T>{this->shared_from_this(), incomingTemporaryMessage_},
                    incomingTemporaryMessage_.header.hasPriority);
                incomingMessages_.push(
                    pair);
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
            mutexSend_.lock();
            std::function<void()> postCallback;
            std::pair<Message<T>, bool> pair = std::make_pair(msg, msg.header.hasPriority);
            outgoingMessages_.push(pair);
            LOG_DBG <<"Adding message to outgoing queue: " << msg;
            if (isWriting)
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
                    threadWrite_ = std::thread([this](){ writeMessages(); });
                };
            }
            isWriting = true;
            mutexSend_.unlock();
            boost::asio::post(context_, postCallback);
        }
    
        void writeMessages()
        {
            while (!outgoingMessages_.empty())
            {
                mutexWrite_.lock();
                const auto& outgoingMessage = outgoingMessages_.pop().first;
                writeHeader(outgoingMessage);
                if (outgoingMessage.header.size > 0)
                {
                    writeBody(outgoingMessage);
                }
                else
                {
                    LOG_DBG << "Finished writing message ";
                }
                mutexWrite_.unlock();
            }
            isWriting = false;
            if (threadWrite_.joinable())
                threadWrite_.join();
        }
    
        void writeHeader(const Message<T>& outgoingMessage)
        {
            boost::system::error_code errcode;
            socket_.write_some(boost::asio::buffer(&outgoingMessage.header, sizeof(MessageHeader<T>)), errcode);
        
            if (errcode)
            {
                LOG_ERR << "Failed to write message header: " << errcode.message();
                disconnect();
                return;
            }
            LOG_DBG << "Finished writing header";
        }
    
        void writeBody(const Message<T>& outgoingMessage)
        {
            boost::system::error_code errcode;
            socket_.write_some(boost::asio::buffer(outgoingMessage.body.data(), outgoingMessage.size()), errcode);
        
            if (errcode)
            {
                LOG_ERR << "Failed to write message body: " << errcode.message();
                disconnect();
                return;
            }
            LOG_DBG << "Finished writing message";
        }
    
        void disconnect()
        {
            if (isConnected())
            {
                joinThreads();
                boost::asio::post(context_, [this]() { socket_.close(); });
            }
        }
    
        void joinThreads()
        {
            if (threadRead_.joinable())
                threadRead_.join();
        
            if (threadWrite_.joinable())
                threadWrite_.join();
        }
    };
}   // namespace net
}   // namespace ipc
#endif // #IPC_NET_CONNECTION_HPP
