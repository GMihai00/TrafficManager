#pragma once
#ifndef MODEL_VEHICLETRAKER_HPP
#define MODEL_VEHICLETRAKER_HPP

#include <optional>
#include <string>
#include <thread>
#include <condition_variable>
#include <memory>
#include <stack>
#include <filesystem>

#include "net/Client.hpp"
#include "net/Client.hpp"
#include "net/Message.hpp"
#include "net/ProxyReply.hpp"
#include "net/ProxyRedirect.hpp"
#include "MessageTypes.hpp"
#include "db/Junction.hpp"
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
        std::thread threadProcess_;
        std::mutex mutexProcess_;
        std::condition_variable condVarProcess_;
        std::atomic_bool shouldPause_ = true;
        bool isRedirected_ = false;

        std::stack<std::pair<ipc::utile::IP_ADRESS, ipc::utile::PORT>> lastVisitedJunctions_;
        std::optional<std::string> signature_;

        bool isEmergency_;
        std::shared_ptr<common::db::Junction> nextJunction_;
        LANE followedLane_;

        LOGGER("VEHICLETRAKER-CLIENT");

        void process();
        bool start();
        void pause();

        bool switchConnectionToRedirectedProxy(ipc::net::ProxyRedirect& redirect);
        bool handleProxyAnswear(ipc::net::Message<ipc::VehicleDetectionMessages>& msg);
        bool queryProxy();
        bool setupData(ipc::net::ProxyReply& reply);
        bool notifyJunction();
        void waitToPassJunction();
    public:
        VehicleTrackerClient() = delete;
        VehicleTrackerClient(std::istream& inputStream);
        ~VehicleTrackerClient();

        // SHOULD HAVE SEPARATE CLASS THAT DOES THIS IN UTILE
        bool saveDataToJson();
    };
}
#endif // #MODEL_VEHICLETRAKER_HPP

