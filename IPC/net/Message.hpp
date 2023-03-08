#pragma once
#ifndef IPC_NET_MESSAGE_HPP
#define IPC_NET_MESSAGE_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <cstdint>
#include <cstring>
#include <memory>

namespace ipc
{
    namespace net
    {
        template <typename T>
        struct MessageHeader
        {
            T type{};
            uint16_t id{};
            bool hasPriority = false;
            size_t size = 0;
        };
    
        template <typename T>
        struct Message
        {
            MessageHeader<T> header{};
            std::vector<uint8_t> body;

            size_t size() const
            {
                return body.size();
            }
        
            friend std::ostream& operator << (std::ostream& os, const Message<T>& msg)
            {
                os  << "ID:" << msg.header.id 
                    << " Size:" << msg.header.size
                    << " HasPriority:" << msg.header.hasPriority
                    << " Type:" << int(msg.header.type);
                return os;
            }
        
            template<typename DataType>
            friend Message<T>& operator << (Message<T>& msg, const DataType& data)
            {
                static_assert(std::is_standard_layout<DataType>::value, "Data can not be pushed");
                size_t sizeBeforePush = msg.body.size();
                msg.body.resize(sizeBeforePush + sizeof(DataType));
                std::memcpy(msg.body.data() + sizeBeforePush, &data, sizeof(DataType));
                msg.header.size = msg.size();
            
                return msg;
            }
        
            template<typename DataType>
            friend Message<T>& operator << (Message<T>& msg, const std::vector<DataType>& dataVec)
            {
                static_assert(std::is_standard_layout<DataType>::value, "Data can not be pushed");
                size_t sizeBeforePush = msg.body.size();
                msg.body.resize(sizeBeforePush + (sizeof(DataType) * dataVec.size()));
                std::memcpy(
                    msg.body.data() + sizeBeforePush,
                    dataVec.data(),
                    (sizeof(DataType) * dataVec.size()));
                msg.header.size = msg.size();
            
                return msg;
            }
        
            template<typename DataType>
            friend Message<T>& operator >> (Message<T>& msg, DataType& data)
            {
                static_assert(std::is_standard_layout<DataType>::value, "Data can not be poped");
             
                if (msg.body.size() < sizeof(DataType))
                {
                    std::cerr << "ERR [MESSAGE] Tried reading data, but it was insufficient\n";
                    return msg;
                }

                size_t sizeAfterPop = msg.body.size() - sizeof(DataType);
                std::memcpy(&data, msg.body.data() + sizeAfterPop, sizeof(DataType));
                msg.body.resize(sizeAfterPop);
                msg.header.size = msg.size();

                return msg;
            }

            template<typename DataType>
            friend Message<T>& operator >> (Message<T>& msg, std::vector<DataType>& dataVec)
            {
                static_assert(std::is_standard_layout<DataType>::value, "Data can not be poped");
               
                if (msg.body.size() < sizeof(DataType) * dataVec.size())
                {
                    std::cerr << "ERR [MESSAGE] Tried reading data, but it was insufficient\n";
                    return msg;
                }

                for (int poz = dataVec.size() - 1; poz >= 0; poz--)
                {
                    msg >> dataVec[poz];
                }
           
                return msg;
            }

            friend Message<T>& operator << (Message<T>& msg, const std::string& data)
            {
                std::vector<char> convertedString(data.begin(), data.end());

                size_t sizeBeforePush = msg.body.size();
                msg.body.resize(sizeBeforePush + sizeof(char) * convertedString.size());
                std::memcpy(msg.body.data() + sizeBeforePush, convertedString.data(), sizeof(char) * convertedString.size());
                msg.header.size = msg.size();

                return msg;
            }


            void clear()
            {
                body.clear();
                header.id = 0;
                header.hasPriority = false;
                header.size = this->size();
            }
        };
    
        template <typename T>
        class Connection;

        template<typename T>
        struct OwnedMessage
        {
            std::shared_ptr<Connection<T>> remote; // WE SEND WHOLE OBJECT OVER THE NETWORK, NOT THE POINTER;
            Message<T> msg;
        
            friend std::ostream& operator << (std::ostream& os, const OwnedMessage<T>& msg)
            {
                os << msg.msg();
                return os;
            }
            OwnedMessage(const std::shared_ptr<Connection<T>>& remotec, const Message<T>& msgc) :
                remote{ remotec },
                msg {msgc}
            {

            }
        };
    } // namespace net
} // namespace ipc
#endif // #IPC_NET_MESSAGE_HPP
