#include "SystemClient.hpp"

namespace model
{
    SystemClient::SystemClient() :
        ipc::net::Client<ipc::VehicleDetectionMessages>(),
        carTracker_(0)
    {
        this->startTrackingCars();
    }

    SystemClient::~SystemClient()
    {
        this->stopTrackingCars();
    }

    bool SystemClient::sendVehicleData(int numberOfCars, bool hasPriority, bool rightLane)
    {
        ipc::net::Message<ipc::VehicleDetectionMessages> message;

        const auto type = ipc::VehicleDetectionMessages::VCDR;
        message.header.type = type;
        message.header.id = messageIdProvider_.provideId(type);
        message.header.hasPriority = hasPriority;
        if (rightLane == true)
            message << 'R';
        else
            message << 'L';
        message << numberOfCars;

        if (connection_)
        {
            connection_->send(message);
        }
        else
        {
            LOG_ERR << "Failed to send message, connection not established";
        }
        return false;
    }

    bool SystemClient::startTrackingCars()
    {
        return carTracker_.startTracking();
    }

    void SystemClient::stopTrackingCars()
    {
        carTracker_.stopTracking();
    }
} // namespace model