#pragma once
#ifndef IPC_NET_PROXYREPLY_HPP
#define IPC_NET_PROXYREPLY_HPP

#include "Message.hpp"
#include "../MessageTypes.hpp"
#include "../utile/IPCDataTypes.hpp"
#include "utile/GeoCoordinate.hpp"
#include "utile/DataTypes.hpp"

namespace ipc
{
    namespace net
    {
        using namespace common::utile;
        class ProxyReply
        {
        private:
            MessageHeader<ipc::VehicleDetectionMessages> header_;
            ipc::utile::IP_ADRESS serverIPAdress_;
            ipc::utile::PORT serverPort_;
            GeoCoordinate<DecimalCoordinate> serverCoordinates_;
            LANE followedLane_;

            void readIpAdress(Message<ipc::VehicleDetectionMessages>& msg);
            void readPort(Message<ipc::VehicleDetectionMessages>& msg);
            void readCoordinates(Message<ipc::VehicleDetectionMessages>& msg);
            void readLane(Message<ipc::VehicleDetectionMessages>& msg);
        public:
            ProxyReply() = delete;
            ProxyReply(MessageHeader<ipc::VehicleDetectionMessages> header,
                ipc::utile::IP_ADRESS serverIPAdress,
                ipc::utile::PORT serverPort,
                GeoCoordinate<DecimalCoordinate> serverCoordinates, 
                LANE followedLane);
            ProxyReply(Message<ipc::VehicleDetectionMessages>& msg);
            operator Message<ipc::VehicleDetectionMessages>() const;
            ~ProxyReply() noexcept = default;

            std::pair<ipc::utile::IP_ADRESS, ipc::utile::PORT> getServerIPAdressAndPort() const;
            bool isEmergency() const;
            GeoCoordinate<DecimalCoordinate> getServerCoordinates() const;
            LANE getFollowedLane() const;
            bool isApproved() const;
        };
    } // namespace net
} // namespace ipc
#endif // #IPC_NET_PROXYREPLY_HPP