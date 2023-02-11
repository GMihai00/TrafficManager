#pragma once
#ifndef MODEL_VEHICLETRAKER_HPP
#define MODEL_VEHICLETRAKER_HPP

#include <optional>
#include <string>

#include "net/Client.hpp"
#include "net/Client.hpp"
#include "net/Message.hpp"
#include "MessageTypes.hpp"
#include "utile/MessageIdProvider.hpp"
#include "utile/Logger.hpp"
#include "utile/Observer.hpp"
#include "utile/GeoCoordinate.hpp";
#include "GPSAdapter.hpp"

namespace model
{
    // THIS CLIENT JUST CONNECTS TO THE PROXY
    // THE PROXY WILL SEND BACK THE COORDINATES OF THE JUNCTION
    // AFTER PASSING TROUGH JUNCTION COORDINATES, CLIENT WILL SEND "LEAVING MESSAGE" TO CLIENT
    // AND WILL REQUERY PROXY FOR NEXT JUNCTION COORDINATES
    class VehicleTrackerClient : public ipc::net::Client<ipc::VehicleDetectionMessages>
    {
    private:
        // signature taken from registry
        // when installing app device registry values
        // are set based on type of account
        std::optional<std::string> signature_;
        ipc::utile::MessageIdProvider<ipc::VehicleDetectionMessages> messageIdProvider_;
        ipc::utile::IP_ADRESS proxyIp_;
        ipc::utile::IP_ADRESS junctionIp_;
        common::utile::GeoCoordinate<double> nextJunctionCoordinates_;
        GPSAdapter gpsAdapter_;
        LOGGER("VEHICLETRAKER-CLIENT");

        bool queryProxyServer();
    public:
        VehicleTrackerClient() = delete;
        VehicleTrackerClient(std::istream& inputStream);
        ~VehicleTrackerClient();
    };
}
#endif // #MODEL_VEHICLETRAKER_HPP

