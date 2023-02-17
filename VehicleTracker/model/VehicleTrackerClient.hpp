#pragma once
#ifndef MODEL_VEHICLETRAKER_HPP
#define MODEL_VEHICLETRAKER_HPP

#include <optional>
#include <string>

#include "net/Client.hpp"
#include "net/Client.hpp"
#include "net/Message.hpp"
#include "MessageTypes.hpp"
#include "utile/DataTypes.hpp"
#include "utile/MessageIdProvider.hpp"
#include "utile/Logger.hpp"
#include "utile/Observer.hpp"
#include "utile/GeoCoordinate.hpp";
#include "GPSAdapter.hpp"

namespace model
{
    using namespace common::utile;
    // THIS CLIENT JUST CONNECTS TO THE PROXY
    // THE PROXY WILL SEND BACK THE COORDINATES OF THE JUNCTION
    // AFTER PASSING TROUGH JUNCTION COORDINATES, CLIENT WILL SEND "LEAVING MESSAGE" TO CLIENT
    // AND WILL REQUERY PROXY FOR NEXT JUNCTION COORDINATES
    class VehicleTrackerClient : public ipc::net::Client<ipc::VehicleDetectionMessages>
    {
    private:
        ipc::utile::MessageIdProvider<ipc::VehicleDetectionMessages> messageIdProvider_;
        GPSAdapter gpsAdapter_;

        std::optional<std::string> signature_; // MAYBE TAKEN FROM REGISTRY
        ipc::utile::IP_ADRESS proxyIp_;

        bool isEmergency_;
        ipc::utile::IP_ADRESS junctionIp_;
        GeoCoordinate<DecimalCoordinate> nextJunctionCoordinates_;
        LANE followedLane_;

        LOGGER("VEHICLETRAKER-CLIENT");

        bool queryProxyServer();
        bool setupData(ipc::net::Message<ipc::VehicleDetectionMessages> msg);
    public:
        VehicleTrackerClient() = delete;
        VehicleTrackerClient(std::istream& inputStream);
        ~VehicleTrackerClient();
    };
}
#endif // #MODEL_VEHICLETRAKER_HPP

