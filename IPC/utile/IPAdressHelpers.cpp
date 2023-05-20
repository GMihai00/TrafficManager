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

	} // namespace utile
} // namespace ipc