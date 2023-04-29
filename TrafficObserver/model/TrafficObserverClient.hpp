#pragma once
#ifndef MODEL_TRAFICOBSERVERCLIENT_HPP
#define MODEL_TRAFICOBSERVERCLIENT_HPP

#include <filesystem>
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
    class TrafficObserverClient : public ipc::net::Client<ipc::VehicleDetectionMessages>
    {
    private:
        ipc::utile::MessageIdProvider<ipc::VehicleDetectionMessages> messageIdProvider_;
        cvision::ObjectTracker carTracker_;
        common::utile::IObserverPtr observer_;
        std::function<void()> observer_callback_;
        std::string keyword_;

        LOGGER("TRAFFICOBSERVER-CLIENT");
        bool startTrackingCars();
        void stopTrackingCars();
        bool sendData(size_t numberOfCars, bool leftLane = false);
    public:
        TrafficObserverClient(std::string keyword, std::filesystem::path videoPath);
        TrafficObserverClient(std::string keyword);
        ~TrafficObserverClient();

        void handleNewCarData();
    };
} // namespace model
#endif // #MODEL_TRAFICOBSERVERCLIENT_HPP
