#include "IPAdressHelpers.hpp"

namespace ipc
{
	namespace utile
	{
		bool IsIPV4(IP_ADRESS adress)
		{
			size_t start = 0;
			size_t end = std::string::npos;
			for (int i = 1; i <= 3; i++)
			{
				start = 0;
				std::string nexValue = "";
				end = adress.find_first_of(".");
				if (end == std::string::npos) { return false; }
				try
				{
					auto value = std::stoi(adress.substr(start, end - start));
					if (!(value >= 0 && value <= 255)) { return false; }
				}
				catch (...)
				{
					return false;
				}
				start = end + 1;
			}
			end = adress.size() - 1;
			try
			{
				auto value = std::stoi(adress.substr(start, end - start + 1));
				if (!(value >= 0 && value <= 255)) { return false; }
			}
			catch (...)
			{
				return false;
			}
			return true;
		}

		// TO DO FOR NOW NOT SUPPORTING IPV6
		bool IsIPV6(IP_ADRESS adress)
		{

			return false;
		}

		// FOR NOW NOT WORKING DUE TO iphlpapi.dll MISSING WILL CHECK LATER THE PROBLEM
		// TAKEN FROM MICROSOFT MOSTLY
		// SUPPORTING ONLY IPV4 FROM WHAT I CAN SEE
		bool ChangeIPAdress(IP_ADRESS address, IP_ADRESS mask)
		{
			#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
			#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

			if (!IsIPV4(address) && !IsIPV4(mask))
			{
				return false;
			}
			in_addr ipv4addr;
			InetPton(AF_INET, PCWSTR(address.c_str()), &ipv4addr);
			IPAddr iaIPAddress = ipv4addr.S_un.S_addr;
			InetPton(AF_INET, PCWSTR(mask.c_str()), &ipv4addr);
			IPMask iaIPMask = ipv4addr.S_un.S_addr;

			if (iaIPAddress == INADDR_NONE || iaIPMask == INADDR_NONE)
			{
				return false;
			}
			// Before calling AddIPAddress we use GetIpAddrTable to get
				// an adapter to which we can add the IP.
			PMIB_IPADDRTABLE pIPAddrTable = (MIB_IPADDRTABLE*)MALLOC(sizeof(MIB_IPADDRTABLE));
			if (pIPAddrTable == NULL) {
				printf("Error allocating memory needed to call GetIpAddrTable\n");
				return false;
			}
			
			DWORD dwSize = 0;
			// Make an initial call to GetIpAddrTable to get the
			// necessary size into the dwSize variable
	//		if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) ==
	//			ERROR_INSUFFICIENT_BUFFER) {
	//			FREE(pIPAddrTable);
	//			pIPAddrTable = (MIB_IPADDRTABLE*)MALLOC(dwSize);

	//		}
	//		if (pIPAddrTable == NULL) {
	//			printf("Memory allocation failed for GetIpAddrTable\n");
	//			return false;
	//		}

	//		 Make a second call to GetIpAddrTable to get the
	//		 actual data we want
	//		DWORD dwRetVal = 0ull;
	//		DWORD ifIndex = 0ull;
	//		if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0)) == NO_ERROR) {
	//			 Save the interface index to use for adding an IP address
	//			ifIndex = pIPAddrTable->table[0].dwIndex;
	//			IN_ADDR IPAddr;
	///*			printf("\n\tInterface Index:\t%ld\n", ifIndex);
	//			IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[0].dwAddr;
	//			printf("\tIP Address:       \t%s (%lu%)\n", inet_ntoa(IPAddr),
	//				pIPAddrTable->table[0].dwAddr);
	//			IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[0].dwMask;
	//			printf("\tSubnet Mask:      \t%s (%lu%)\n", inet_ntoa(IPAddr),
	//				pIPAddrTable->table[0].dwMask);
	//			IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[0].dwBCastAddr;
	//			printf("\tBroadCast Address:\t%s (%lu%)\n", inet_ntoa(IPAddr),
	//				pIPAddrTable->table[0].dwBCastAddr);
	//			printf("\tReassembly size:  \t%lu\n\n",
	//				pIPAddrTable->table[0].dwReasmSize);*/
	//		}
	//		else {
	//			printf("Call to GetIpAddrTable failed with error %d.\n", dwRetVal);
	//			if (pIPAddrTable)
	//				FREE(pIPAddrTable);
	//			return false;
	//		}

	//		if (pIPAddrTable) {
	//			FREE(pIPAddrTable);
	//			pIPAddrTable = NULL;
	//		}

	//		ULONG NTEContext = 0;
	//		ULONG NTEInstance = 0;
	//		if ((dwRetVal = AddIPAddress(iaIPAddress, iaIPMask, ifIndex, &NTEContext, &NTEInstance)) != NO_ERROR)
	//		{
	//			return false;
	//		}

			return true;
		}
	} // namespace utile
} // namespace ipc