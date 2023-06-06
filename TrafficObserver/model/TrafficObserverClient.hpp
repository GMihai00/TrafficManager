#pragma once
#ifndef MODEL_TRAFICOBSERVERCLIENT_HPP
#define MODEL_TRAFICOBSERVERCLIENT_HPP

#include <filesystem>
#include <utility>
#include <condition_variable>

#include <cryptopp870/dll.h>
#include <cryptopp870/rsa.h>
#include <cryptopp870/osrng.h>
#include <cryptopp870/base64.h>

#include "net/Client.hpp"
#include "net/Message.hpp"
#include "MessageTypes.hpp"
#include "utile/MessageIdProvider.hpp"
#include "cvision/ObjectTracker.hpp"
#include "utile/Logger.hpp"
#include "utile/Observer.hpp"
#include "utile/ConfigHelpers.hpp"

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
        CryptoPP::AutoSeededRandomPool rng_;
        std::optional<CryptoPP::RSA::PublicKey> publicKey_ = std::nullopt;
        std::string encryptedKey; // crash when this is dealocated
        std::atomic_bool secureConnectionEstablished_ = false;
        std::mutex mutexSendCarData_;

        size_t carCountLeft_ = 0;
        size_t carCountRight_ = 0;

        LOGGER("TRAFFICOBSERVER-CLIENT");
        bool startTrackingCars();
        void stopTrackingCars();
        bool sendCarData(const size_t numberOfCars, const  uint8_t leftLane = 0);
        bool requestPublicKey();
        bool sendSecureConnectRequest();

    public:
        TrafficObserverClient(const std::string& keyword, const std::filesystem::path& videoPath);
        TrafficObserverClient(const std::string& keyword);
        ~TrafficObserverClient();

        void handleNewCarData();

        bool secureConnect(const common::utile::IP_ADRESS& host, const ipc::utile::PORT port);
    };
} // namespace model
#endif // #MODEL_TRAFICOBSERVERCLIENT_HPP
