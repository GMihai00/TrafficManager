#include <filesystem>

#include "utile/Logger.hpp"
#include "utile/IPCDataTypes.hpp"
#include "utile/GeoCoordinate.hpp"

LOGGER("TEST-MAIN");

// Need to read JMS config to enable this. Might move configLoader to common 
bool tryToRun4TOsForEachJMS(const ipc::utile::IP_ADRESS& serverIp, const ipc::utile::PORT& port, const std::string& keyword)
{

}

// could have all the proxys in a config file
bool runProxy(const ipc::utile::IP_ADRESS& serverIp, 
	const ipc::utile::PORT& port, 
	common::utile::GeoCoordinate<DecimalCoordinate> boundSW,
	common::utile::GeoCoordinate<DecimalCoordinate> boundNe,
	std::string dbServer,
	std::string dbusername,
	std::string dbpassword)
{

}

bool runJMSForAllConfigs(const std::filesystem::path& jmsConfigDir)
{
	
}

// Data generate with the help of https://www.nmeagen.org/
bool runVTForEachGPS(const std::filesystem::path& gpsDataDir, const std::filesystem::path& configPath)
{

}

int main()
{

	return 0;
}