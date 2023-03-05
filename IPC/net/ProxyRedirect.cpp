#include "ProxyRedirect.hpp"
#include <exception>

namespace ipc
{
    namespace net
    {
        void ProxyRedirect::readIpAdress(Message<ipc::VehicleDetectionMessages>& msg)
        {
            // IP (IPV4) - char[9]
            std::vector<char> ip;
            ip.resize(9);
            msg >> ip;
            serverIPAdress_ = std::string(ip.begin(), ip.end());
            //VALIDATE DATA THROW EXCEPTION IF INVALID
            if (!ipc::utile::IsIPV4(serverIPAdress_))
            {
                throw std::runtime_error("Invalid ProxyRedirect");
            }
        }

        void ProxyRedirect::readPort(Message<ipc::VehicleDetectionMessages>& msg)
        {
            ipc::utile::PORT port = std::numeric_limits<std::uint16_t>::max();
            msg >> port;

            if (port == std::numeric_limits<std::uint16_t>::max()) { throw std::runtime_error("Invalid ProxyRedirect"); }
            serverPort_ = port;
        }

        ProxyRedirect::ProxyRedirect(Message<ipc::VehicleDetectionMessages>& msg)
        {
            readPort(msg);
            readIpAdress(msg);
            
            this->header_ = msg.header;
        }

        ProxyRedirect::operator Message<ipc::VehicleDetectionMessages>() const
        {
            Message<ipc::VehicleDetectionMessages> message;
            message.header = this->header_;
            std::vector<char> ip(serverIPAdress_.begin(), serverIPAdress_.end());
            message << ip;
            message << serverPort_;
            return message;
        }

        std::pair<ipc::utile::IP_ADRESS, ipc::utile::PORT> ProxyRedirect::getServerIPAdressAndPort() const
        {
            return { serverIPAdress_, serverPort_ };
        }

        bool ProxyRedirect::isEmergency() const
        {
            return header_.hasPriority;
        }


        bool ProxyRedirect::isApproved() const
        {
            return (header_.type == ipc::VehicleDetectionMessages::ACK);
        }

    } // namespace net
} // namespace ipc