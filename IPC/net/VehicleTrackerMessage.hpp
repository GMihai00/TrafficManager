#pragma once
#ifndef IPC_NET_VEHICLETRACKERMESSAGE_HPP
#define IPC_NET_VEHICLETRACKERMESSAGE_HPP

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
        class VehicleTrackerMessage
        {
        private:
            MessageHeader<ipc::VehicleDetectionMessages> header_;
            GeoCoordinate<DecimalCoordinate> pointACoordinates_;
            GeoCoordinate<DecimalCoordinate> pointBCoordinates_;

            void readCoordinates(Message<ipc::VehicleDetectionMessages>& msg);
        public:
            VehicleTrackerMessage() = delete;
            VehicleTrackerMessage(Message<ipc::VehicleDetectionMessages>& msg);
            operator Message<ipc::VehicleDetectionMessages>() const;
            ~VehicleTrackerMessage() noexcept = default;

            bool isApproved() const;
            std::pair<GeoCoordinate<DecimalCoordinate>, GeoCoordinate<DecimalCoordinate>> getCoordinates() const;
        };
    } // namespace net
} // namespace ipc
#endif // #IPC_NET_VEHICLETRACKERMESSAGE_HPP