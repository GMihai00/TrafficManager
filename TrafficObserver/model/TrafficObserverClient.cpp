#include "TrafficObserverClient.hpp"

namespace model
{
    void TrafficObserverClient::handleNewCarData()
    {
        auto carCount = carTracker_.getCarCount();
        carTracker_.resetCarCount();

        sendData(carCount.first, true);
        sendData(carCount.second);
    }

    TrafficObserverClient::TrafficObserverClient(std::string keyword) :
        ipc::net::Client<ipc::VehicleDetectionMessages>(),
        keyword_(keyword),
        carTracker_(0)
    {
        observer_callback_ = std::bind(&TrafficObserverClient::handleNewCarData, this);
        observer_ = std::make_shared<common::utile::Observer>(observer_callback_);
        carTracker_.subscribe(observer_);
        this->startTrackingCars();
    }

    TrafficObserverClient::TrafficObserverClient(std::string keyword, std::filesystem::path videoPath) :
        ipc::net::Client<ipc::VehicleDetectionMessages>(),
        keyword_(keyword),
        carTracker_(videoPath.string())
    {
        observer_callback_ = std::bind(&TrafficObserverClient::handleNewCarData, this);
        observer_ = std::make_shared<common::utile::Observer>(observer_callback_);
        carTracker_.subscribe(observer_);
        this->startTrackingCars();
    }

    TrafficObserverClient::~TrafficObserverClient()
    {
        this->stopTrackingCars();
    }

    bool TrafficObserverClient::sendData(size_t numberOfCars, bool leftLane)
    {
        if (!connection_)
        {
            LOG_ERR << "Connection not established, failed to send message";
            return false;
        }

        for (size_t i = 0; i < numberOfCars; i++)
        {
            ipc::net::Message<ipc::VehicleDetectionMessages> message;
            message.header.type = ipc::VehicleDetectionMessages::VCDR;
            message.header.id = messageIdProvider_.provideId(ipc::VehicleDetectionMessages::VCDR);
            message.header.hasPriority = false;
            message << leftLane;
            message << keyword_;
            connection_->send(message);
        }

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