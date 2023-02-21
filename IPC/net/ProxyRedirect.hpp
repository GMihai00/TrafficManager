#pragma once
#ifndef IPC_NET_PROXYREDIRECT_HPP
#define IPC_NET_PROXYREDIRECT_HPP

#include "Message.hpp"
#include "../MessageTypes.hpp"
#include "../utile/IPAdressHelpers.hpp"

#include "utile/DataTypes.hpp"
#include "utile/GeoCoordinate.hpp"

namespace ipc
{
    namespace net
    {
        using namespace common::utile;
        class ProxyRedirect
        {
        private:
            MessageHeader<ipc::VehicleDetectionMessages> header_;
            ipc::utile::IP_ADRESS serverIPAdress_;
            ipc::utile::PORT serverPort_;

            void readIpAdress(Message<ipc::VehicleDetectionMessages>& msg);
            void readPort(Message<ipc::VehicleDetectionMessages>& msg);
        public:
            ProxyRedirect() = delete;
            ProxyRedirect(Message<ipc::VehicleDetectionMessages>& msg);
            operator Message<ipc::VehicleDetectionMessages>() const;
            ~ProxyRedirect() noexcept = default;

            std::pair<ipc::utile::IP_ADRESS, ipc::utile::PORT> getServerIPAdressAndPort() const;
            bool isEmergency() const;
            bool isApproved() const;
        };
    } // namespace net
} // namespace ipc
#endif // #IPC_NET_PROXYREDIRECT_HPP