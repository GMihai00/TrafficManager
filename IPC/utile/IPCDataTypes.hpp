#pragma once
#ifndef IPC_UTILE_DATATYPES_HPP
#define IPC_UTILE_DATATYPES_HPP

#include <string>
#include <set>
#include "../MessageTypes.hpp"
namespace ipc
{
	namespace utile
	{
		typedef uint16_t PORT;
		typedef std::string IP_ADRESS;
		typedef std::set<IP_ADRESS> IP_ADRESSES;
		typedef std::shared_ptr < ipc::net::Connection<ipc::VehicleDetectionMessages>> ConnectionPtr;
		typedef ipc::net::Message< ipc::VehicleDetectionMessages> VehicleDetectionMessage;
	}
}
#endif // #IPC_UTILE_DATATYPES_HPP