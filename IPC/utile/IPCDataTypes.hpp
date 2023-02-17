#pragma once
#ifndef IPC_UTILE_DATATYPES_HPP
#define IPC_UTILE_DATATYPES_HPP

#include <string>
#include <set>
#include <memory>
#include "../MessageTypes.hpp"
#include "../net/Connection.hpp"
#include "../net/Message.hpp"

namespace ipc
{
	namespace utile
	{
		#define G_PROXY_IP "255.255.255.255";
		#define G_PROXY_PORT 900;

		typedef uint16_t PORT;
		typedef std::string IP_ADRESS;
		typedef std::set<IP_ADRESS> IP_ADRESSES;
		typedef std::shared_ptr < ipc::net::Connection<ipc::VehicleDetectionMessages>> ConnectionPtr;
		typedef ipc::net::Message< ipc::VehicleDetectionMessages> VehicleDetectionMessage;

		bool IsIPV4(IP_ADRESS adress);
		bool IsIPV6(IP_ADRESS adress);
	} // namespace utile
} // namespace ipc
#endif // #IPC_UTILE_DATATYPES_HPP