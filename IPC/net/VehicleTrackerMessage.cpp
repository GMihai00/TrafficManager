#include "VehicleTrackerMessage.hpp"
#include <exception>

namespace ipc
{
    namespace net
    {

        void VehicleTrackerMessage::readCoordinates(Message<ipc::VehicleDetectionMessages>& msg)
        {
            // COORDINATES { latitude, longitude } - double
            DecimalCoordinate latA = DECIMALCOORDINATE_INVALID_VALUE;
            msg >> latA;
            if (latA == DECIMALCOORDINATE_INVALID_VALUE) { throw std::runtime_error("Invalid ProxyReply"); }
            DecimalCoordinate lonA = DECIMALCOORDINATE_INVALID_VALUE;
            msg >> lonA;
            if (lonA == DECIMALCOORDINATE_INVALID_VALUE) { throw std::runtime_error("Invalid ProxyReply"); }

            pointACoordinates_.latitude = latA;
            pointACoordinates_.longitude = lonA;

            DecimalCoordinate latB = DECIMALCOORDINATE_INVALID_VALUE;
            msg >> latB;
            if (latB == DECIMALCOORDINATE_INVALID_VALUE) { throw std::runtime_error("Invalid ProxyReply"); }
            DecimalCoordinate lonB = DECIMALCOORDINATE_INVALID_VALUE;
            msg >> lonB;
            if (lonB == DECIMALCOORDINATE_INVALID_VALUE) { throw std::runtime_error("Invalid ProxyReply"); }

            pointBCoordinates_.latitude = latB;
            pointBCoordinates_.longitude = lonB;
        }


        VehicleTrackerMessage::VehicleTrackerMessage(Message<ipc::VehicleDetectionMessages>& msg)
        {
            readCoordinates(msg);
            this->header_ = msg.header;
        }

        VehicleTrackerMessage::operator Message<ipc::VehicleDetectionMessages>() const
        {
            Message<ipc::VehicleDetectionMessages> message;
            message.header = this->header_;
            message << pointACoordinates_.latitude << pointACoordinates_.longitude;
            message << pointBCoordinates_.latitude << pointBCoordinates_.longitude;
            return message;
        }


        std::pair< GeoCoordinate<DecimalCoordinate>, GeoCoordinate<DecimalCoordinate>> VehicleTrackerMessage::getCoordinates() const
        {
            return { pointACoordinates_, pointBCoordinates_ };
        }

        bool VehicleTrackerMessage::isApproved() const
        {
            return (header_.type == ipc::VehicleDetectionMessages::ACK);
        }

    } // namespace net
} // namespace ipc