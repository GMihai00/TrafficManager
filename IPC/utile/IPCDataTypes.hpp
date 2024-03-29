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
		typedef uint16_t PORT;
		typedef std::string IP_ADRESS;
		typedef std::set<IP_ADRESS> IP_ADRESSES;
		typedef std::shared_ptr < ipc::net::Connection<ipc::VehicleDetectionMessages>> ConnectionPtr;
		typedef ipc::net::Message< ipc::VehicleDetectionMessages> VehicleDetectionMessage;
	} // namespace utile
} // namespace ipc
#endif // #IPC_UTILE_DATATYPES_HPP