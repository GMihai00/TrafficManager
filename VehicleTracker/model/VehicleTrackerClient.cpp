#include "VehicleTrackerClient.hpp"

namespace model
{
    void VehicleTrackerClient::handleNewCarData()
    {
        auto carCount = carTracker_.getCarCount();
        sendData(carCount.first);
        sendData(carCount.second);
    }

    VehicleTrackerClient::VehicleTrackerClient() :
        ipc::net::Client<ipc::VehicleDetectionMessages>(),
        carTracker_(0)
    {
        observer_ = std::make_shared<common::utile::Observer>(std::bind(&VehicleTrackerClient::handleNewCarData, this));
        carTracker_.subscribe(observer_);
        this->startTrackingCars();
    }

    VehicleTrackerClient::~VehicleTrackerClient()
    {
        this->stopTrackingCars();
    }

    // NEED TO CHANGE THIS TO BE DONE ON SEPARATE THREAD(S)
    bool VehicleTrackerClient::sendData(size_t numberOfCars, bool leaving)
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
            message << leaving;
            connection_->send(message);
        }

        return true;
    }

    bool VehicleTrackerClient::startTrackingCars()
    {
        return carTracker_.startTracking();
    }

    void VehicleTrackerClient::stopTrackingCars()
    {
        carTracker_.stopTracking();
    }
} // namespace model