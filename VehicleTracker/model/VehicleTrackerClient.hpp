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

namespace model
{
    // THIS CLIENT JUST CONNECTS TO THE PROXY
    // THE PROXY WILL SEND BACK THE COORDINATES OF THE JUNCTION
    // AFTER PASSING TROUGH JUNCTION COORDINATES, CLIENT WILL SEND "LEAVING MESSAGE" TO CLIENT
    // AND WILL REQUERY PROXY FOR NEXT JUNCTION COORDINATES
    class VehicleTrackerClient : public ipc::net::Client<ipc::VehicleDetectionMessages>
    {
    private:
        ipc::utile::MessageIdProvider<ipc::VehicleDetectionMessages> messageIdProvider_;

        // signature taken from registry
        // when installing app device registry values
        // are set based on type of account
        std::optional<std::string> signature;
        common::utile::GeoCoordinate nextJunctionCoordinates;
        LOGGER("VEHICLETRAKER-CLIENT");
    public:
        VehicleTrackerClient();
        ~VehicleTrackerClient();
    };
}
#endif // #MODEL_VEHICLETRAKER_HPP

