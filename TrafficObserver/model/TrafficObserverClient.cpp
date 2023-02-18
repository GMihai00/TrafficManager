#include "TrafficObserverClient.hpp"

namespace model
{
    void TrafficObserverClient::handleNewCarData()
    {
        auto carCount = carTracker_.getCarCount();
        if (!usingRightLane_)
        {
            sendData(carCount.first);
        }
        else
        {
            sendData(carCount.second);
        }
    }

    TrafficObserverClient::TrafficObserverClient() :
        ipc::net::Client<ipc::VehicleDetectionMessages>(),
        carTracker_(0)
    {
        observer_ = std::make_shared<common::utile::Observer>(std::bind(&TrafficObserverClient::handleNewCarData, this));
        carTracker_.subscribe(observer_);
        this->startTrackingCars();
    }

    TrafficObserverClient::~TrafficObserverClient()
    {
        this->stopTrackingCars();
    }

    bool TrafficObserverClient::sendData(size_t numberOfCars)
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
            connection_->send(message);
        }

        return true;
    }

    bool TrafficObserverClient::startTrackingCars()
    {
        return carTracker_.startTracking();
    }

    void TrafficObserverClient::stopTrackingCars()
    {
        carTracker_.stopTracking();
    }
} // namespace model