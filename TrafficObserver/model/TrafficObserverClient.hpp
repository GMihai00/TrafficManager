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
#include "utile/ConfigHelpers.hpp"
#include "RSA.hpp"


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
        std::shared_ptr<security::RSA::PublicKey> publicKey_ = nullptr;
        std::atomic_bool secureConnectionEstablished_ = false;
        std::mutex mutexSendCarData_;

        size_t carCountLeft_ = 0;
        size_t carCountRight_ = 0;

        LOGGER("TRAFFICOBSERVER-CLIENT");
        bool startTrackingCars();
        void stopTrackingCars();
        bool sendCarData(size_t numberOfCars, uint8_t leftLane = 0);
        bool requestPublicKey();
        bool sendSecureConnectRequest();

    public:
        TrafficObserverClient(std::string keyword, std::filesystem::path videoPath);
        TrafficObserverClient(std::string keyword);
        ~TrafficObserverClient();

        void handleNewCarData();

        bool secureConnect(const common::utile::IP_ADRESS& host, const ipc::utile::PORT port);
    };
} // namespace model
#endif // #MODEL_TRAFICOBSERVERCLIENT_HPP
