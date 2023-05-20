#pragma once
#ifndef IPC_UTILE_IPADRESSHELPERS_HPP
#define IPC_UTILE_IPADRESSHELPERS_HPP

#include "IPCDataTypes.hpp"
#include "iphlpapi.h"

namespace ipc
{
	namespace utile
	{
		constexpr auto G_PROXY_IP = "127.0.0.1";
		constexpr PORT G_PROXY_PORT = 6000;
		constexpr auto G_NETMASK = "255.0.0.0";

		bool IsIPV4(IP_ADRESS adress);
		bool IsIPV6(IP_ADRESS adress);
	} // namespace utile
} // namespace ipc
#endif // #IPC_UTILE_IPADRESSHELPERS_HPP