#ifndef IPC_NET_MESSAGETYPES_HPP
#define IPC_NET_MESSAGETYPES_HPP

#include <iostream>

namespace ipc
{
    enum class VehicleDetectionMessages : uint8_t
    {
        ACK, // for connection, it contains coordinates
        NACK,
        VDB, // Vehicle Data Broadcast - latitude longitude, latitude longitude
        VCDR, // Vehicle Client Detection Result - number of vehicles detected
        REDIRECT // FOR PROXY REDIRECTION
    };
} // namespace ipc
#endif // #IPC_NET_MESSAGETYPES_HPP