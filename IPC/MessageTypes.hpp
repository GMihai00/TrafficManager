#ifndef IPC_NET_MESSAGETYPES
#define IPC_NET_MESSAGETYPES

#include <iostream>

namespace ipc
{
    enum class VehicleDetectionMessages : uint8_t
    {
        ACK, // for connection, it contains coordinates
        NACK,
        VDB, // Vehicle Data Broadcast - speed?, latitude, longitude, direction
        VCDR // Vehicle Client Detection Result - number of vehicles detected
    };
} // namespace ipc
#endif // #IPC_NET_MESSAGETYPES