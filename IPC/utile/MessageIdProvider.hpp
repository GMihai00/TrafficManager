#ifndef IPC_UTILE_MESSAGEIDPROVIDER
#define IPC_UTILE_MESSAGEIDPROVIDER

#include "../MessageTypes.hpp"
#include <map>

namespace ipc
{
namespace utile
{
    template <typename T>
    class MessageIdProvider
    {
    private:
        std::map<T, uint16_t> messageToId;
    public:
        MessageIdProvider() = default;
        MessageIdProvider(MessageIdProvider&) = delete;
        ~MessageIdProvider() = default;
    
        uint16_t provideId(T type)
        {
            if (messageToId.find(type) == messageToId.end())
                messageToId[type] = 0;
            return messageToId[type]++;
        }
    };
} // namespace utile
} // namespace ipc
#endif // #IPC_UTILE_MESSAGEIDPROVIDER