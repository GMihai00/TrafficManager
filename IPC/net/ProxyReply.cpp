#include "ProxyReply.hpp"
#include <exception>

namespace ipc
{
    namespace net
    {
        void ProxyReply::readIpAdress(Message<ipc::VehicleDetectionMessages>& msg)
        {
            // IP (IPV4/IPV6) - char[40]
            std::vector<char> ip;
            ip.resize(40);
            msg >> ip;
            serverIPAdress_ = std::string(ip.begin(), ip.end());
            //VALIDATE DATA THROW EXCEPTION IF INVALID
            if (!ipc::utile::IsIPV4(serverIPAdress_) && 
                !ipc::utile::IsIPV6(serverIPAdress_)) 
            { 
                throw std::runtime_error("Invalid ProxyReply");
            }
        }

        void ProxyReply::readPort(Message<ipc::VehicleDetectionMessages>& msg)
        {
            ipc::utile::PORT port = std::numeric_limits<std::uint16_t>::max();
            msg >> port;

            if (port == std::numeric_limits<std::uint16_t>::max()) { throw std::runtime_error("Invalid ProxyReply"); }
            serverPort_ = port;
        }

        void ProxyReply::readCoordinates(Message<ipc::VehicleDetectionMessages>& msg)
        {
            // COORDINATES { latitude, longitude } - double
            DecimalCoordinate lat = DECIMALCOORDINATE_INVALID_VALUE;
            msg >> lat;
            if (lat == DECIMALCOORDINATE_INVALID_VALUE) { throw std::runtime_error("Invalid ProxyReply"); }
            DecimalCoordinate lon = DECIMALCOORDINATE_INVALID_VALUE;
            msg >> lon;
            if (lon == DECIMALCOORDINATE_INVALID_VALUE) { throw std::runtime_error("Invalid ProxyReply"); }

            serverCoordinates_.latitude = lat;
            serverCoordinates_.longitude = lon;
        }


        ProxyReply::ProxyReply(Message<ipc::VehicleDetectionMessages>& msg)
        {   
            readIpAdress(msg);
            readPort(msg);
            readCoordinates(msg);
            this->header_ = msg.header;
        }

        ProxyReply::operator Message<ipc::VehicleDetectionMessages>() const
        {
            Message<ipc::VehicleDetectionMessages> message;
            message.header = this->header_;
            std::vector<char> ip(serverIPAdress_.begin(), serverIPAdress_.end());
            message << ip;
            message << serverPort_;
            message << serverCoordinates_.latitude << serverCoordinates_.longitude;
            return message;
        }

        std::pair<ipc::utile::IP_ADRESS, ipc::utile::PORT> ProxyReply::getServerIPAdressAndPort() const
        {
            return { serverIPAdress_, serverPort_ };
        }

        bool ProxyReply::isEmergency() const
        {
            return header_.hasPriority;
        }

        GeoCoordinate<DecimalCoordinate> ProxyReply::getServerCoordinates() const
        {
            return serverCoordinates_;
        }

        bool ProxyReply::isApproved() const
        {
            return (header_.type == ipc::VehicleDetectionMessages::ACK);
        }

    } // namespace net
} // namespace ipc