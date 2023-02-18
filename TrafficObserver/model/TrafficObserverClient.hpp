#pragma once
#ifndef MODEL_COSTUMCLIENT_HPP
#define MODEL_COSTUMCLIENT_HPP

#include <utility>
#include <condition_variable>

#include "net/Client.hpp"
#include "net/Message.hpp"
#include "MessageTypes.hpp"
#include "utile/MessageIdProvider.hpp"
#include "cvision/ObjectTracker.hpp"
#include "utile/Logger.hpp"
#include "utile/Observer.hpp"

namespace model
{
    // CONFIG FOR RIGHTLANE SCENARIO AND IP ADRESS
    class TrafficObserverClient : public ipc::net::Client<ipc::VehicleDetectionMessages>
    {
    private:
        ipc::utile::MessageIdProvider<ipc::VehicleDetectionMessages> messageIdProvider_;
        cvision::ObjectTracker carTracker_;
        common::utile::IObserverPtr observer_;
        
        bool usingRightLane_ = false; // FOR NOW JUST DEFAULTED
        std::optional<std::string> signature_; // REALIZE THAT I NEED THIS FOR ALL CONNECTIONS!!!

        LOGGER("TRAFFICOBSERVER-CLIENT");
        bool startTrackingCars();
        void stopTrackingCars();
        bool sendData(size_t numberOfCars);
    public:
        TrafficObserverClient();
        ~TrafficObserverClient();

        void handleNewCarData();
    };
} // namespace model
#endif // #MODEL_COSTUMCLIENT_HPP
