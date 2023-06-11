#include "TrafficObserverClient.hpp"

#include "finally.h"

namespace model
{
    void TrafficObserverClient::handleNewCarData()
    {
        auto carCount = carTracker_.getCarCount();

        LOG_INF << "Handling new car data carcount_left: " << carCount.first << " carcount_right: " << carCount.second;

        // from the camera perspective left cars are incoming vehicles aka cars from the right lane
        std::scoped_lock lock(mutexSendCarData_);
        sendCarData(carCount.first - carCountLeft_, 0);
        sendCarData(carCount.second - carCountRight_, 1);

        carCountLeft_ = carCount.first;
        carCountRight_ = carCount.second;
    }

    TrafficObserverClient::TrafficObserverClient(const std::string& keyword) :
        ipc::net::Client<ipc::VehicleDetectionMessages>(),
        keyword_(keyword),
        carTracker_(0)
    {
        observer_callback_ = std::bind(&TrafficObserverClient::handleNewCarData, this);
        observer_ = std::make_shared<common::utile::Observer>(observer_callback_);
        carTracker_.subscribe(observer_);
    }

    TrafficObserverClient::TrafficObserverClient(const std::string& keyword, const std::filesystem::path& videoPath) :
        ipc::net::Client<ipc::VehicleDetectionMessages>(),
        keyword_(keyword),
        carTracker_(videoPath.string())
    {
        observer_callback_ = std::bind(&TrafficObserverClient::handleNewCarData, this);
        observer_ = std::make_shared<common::utile::Observer>(observer_callback_);
        carTracker_.subscribe(observer_);
    }

    TrafficObserverClient::~TrafficObserverClient()
    {
        this->stopTrackingCars();
    }

    bool TrafficObserverClient::secureConnect(const common::utile::IP_ADRESS& host, const ipc::utile::PORT port)
    {
        if (!connect(host, port))
        {
            LOG_ERR << "Failed to connect to the server";
            return false;
        }

        auto cleanupAction = common::utile::finally([this]() { if(!secureConnectionEstablished_) disconnect(); });

        if (!requestPublicKey()) { LOG_ERR << "Failed to acquire public key"; return false; }

        if (!sendSecureConnectRequest()) { LOG_ERR << "Failed to send secure connect req"; return false; }

        if (!waitForAnswear(5000)) { return false; }

        auto answear = getLastUnreadAnswear();

        if (!answear.has_value()) { return false; }

        if (answear.value().first.msg.header.type != ipc::VehicleDetectionMessages::ACK)
            return false;

        secureConnectionEstablished_ = true;
        
        startTrackingCars();

        return true;
    }

    bool TrafficObserverClient::requestPublicKey()
    {
        if (!connection_)
        {
            LOG_ERR << "Connection not established, failed to send message";
            return false;
        }

        ipc::net::Message<ipc::VehicleDetectionMessages> message;
        message.header.type = ipc::VehicleDetectionMessages::PUBLIC_KEY_REQ;
        message.header.id = messageIdProvider_.provideId(ipc::VehicleDetectionMessages::PUBLIC_KEY_REQ);
        message.header.hasPriority = false;

        connection_->send(message);

        if (!waitForAnswear(5000)) { return false; }

        auto answear = getLastUnreadAnswear();

        if (!answear.has_value()) { return false; }

        auto msg = answear.value().first.msg;

        if (msg.header.type != ipc::VehicleDetectionMessages::ACK)
            return false;

        std::string publicKey;
        publicKey.resize(msg.header.size);

        msg >> publicKey;

        try
        {
            m_RSAWrapper = std::make_shared<security::RSAWrapper>(publicKey);
        }
        catch (const std::exception& err)
        {
            LOG_ERR << err.what();
            return false;
        }

        return true;
    }

    bool TrafficObserverClient::sendSecureConnectRequest()
    {
        if (!connection_ || !m_RSAWrapper)
        {
            LOG_ERR << "Connection not established, failed to send message";
            return false;
        }

        ipc::net::Message<ipc::VehicleDetectionMessages> message;
        message.header.type = ipc::VehicleDetectionMessages::SECURE_CONNECT;
        message.header.id = messageIdProvider_.provideId(ipc::VehicleDetectionMessages::SECURE_CONNECT);
        message.header.hasPriority = false;

        auto encryptedKey = m_RSAWrapper->encryptMessage(keyword_);
        
        if (!encryptedKey)
        {
            LOG_ERR << "Failed to encrypt message";
            return false;
        }

        message << *encryptedKey;

        connection_->send(message);

        return true;
    }

    bool TrafficObserverClient::sendCarData(const size_t numberOfCars, const uint8_t leftLane)
    {
        if (!secureConnectionEstablished_ || !connection_)
        {
            LOG_ERR << "Secure connection not established, failed to send message";
            return false;
        }

        LOG_INF << "-------------------------------Sending car data to the JMS---------------------------------------";
        ipc::net::Message<ipc::VehicleDetectionMessages> message;
        message.header.type = ipc::VehicleDetectionMessages::VCDR;
        message.header.id = messageIdProvider_.provideId(ipc::VehicleDetectionMessages::VCDR);
        message.header.hasPriority = false;
        message << leftLane;
        message << numberOfCars;

        connection_->send(message);

        return true;
    }

    bool TrafficObserverClient::startTrackingCars()
    {
        return carTracker_.startTracking(true);
    }

    void TrafficObserverClient::stopTrackingCars()
    {
        carTracker_.stopTracking();
    }
} // namespace model