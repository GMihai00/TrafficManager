#include "TrafficObserverClient.hpp"

namespace model
{
    void TrafficObserverClient::handleNewCarData()
    {
        auto carCount = carTracker_.getCarCount();

        LOG_INF << "Handling new car data carcount_left: " << carCount.first << " carcount_right: " << carCount.second;

        sendData(carCount.first - carCountLeft_, 1);
        sendData(carCount.second - carCountRight_, 0);

        carCountLeft_ = carCount.first;
        carCountRight_ = carCount.second;
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

    bool TrafficObserverClient::sendData(size_t numberOfCars, uint8_t leftLane)
    {
        if (!connection_)
        {
            LOG_ERR << "Connection not established, failed to send message";
            return false;
        }

        for (size_t i = 0; i < numberOfCars; i++)
        {
            LOG_INF << "-------------------------------Sending car data to the JMS---------------------------------------";
            ipc::net::Message<ipc::VehicleDetectionMessages> message;
            message.header.type = ipc::VehicleDetectionMessages::VCDR;
            message.header.id = messageIdProvider_.provideId(ipc::VehicleDetectionMessages::VCDR);
            message.header.hasPriority = false;
            message << leftLane;

            if (!already_sent_keyword_)
            {
                message << keyword_;
                already_sent_keyword_ = true;
            }

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