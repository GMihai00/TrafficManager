#ifndef IPC_NET_MESSAGE
#define IPC_NET_MESSAGE

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
        uint32_t size = 0;
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
            try
            {
                size_t sizeAfterPop = msg.body.size() - sizeof(DataType);
                std::memcpy(&data, msg.body.data() + sizeAfterPop, sizeof(DataType));
                msg.body.resize(sizeAfterPop);
                msg.header.size = msg.size();
            }
            catch(const std::exception& e)
            {
                std::cerr << "Error while extracting data" << e.what() << '\n';
            }
            
            return msg;
        }
        
        template<typename DataType>
        friend Message<T>& operator >> (Message<T>& msg, std::vector<DataType>& dataVec)
        {
            static_assert(std::is_standard_layout<DataType>::value, "Data can not be poped");
            try
            {
                size_t sizeAfterPop = msg.body.size() - sizeof(DataType) * dataVec.size();
                std::memcpy(
                    dataVec.data(),
                    msg.body.data() + sizeAfterPop,
                    sizeof(DataType) * dataVec.size());
                msg.body.resize(sizeAfterPop);
                msg.header.size = msg.size();
            }
            catch(const std::exception& e)
            {
                std::cerr << "Error while extracting data" << e.what() << '\n';
            }
            
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
        std::shared_ptr<Connection<T>> remote = nullptr;
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
#endif // #IPC_NET_MESSAGE
