#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <memory>

#include "model/VehicleTrackerClient.hpp"

#include "utile/CommandLineParser.hpp"
#include "utile/Logger.hpp"
#include "utile/SignalHandler.hpp"

using namespace common::utile;
using namespace common;

LOGGER("MAIN");

// Need to make it global due to signal handling 
std::unique_ptr<model::VehicleTrackerClient> g_vtClient;
std::condition_variable g_condVarEnd;

int main()
{
	SignalHandler sigHandler{};
	sigHandler.setAction(SIGINT, [](int singal)
		{
			if (!(g_vtClient && g_vtClient->saveDataToJson()))
			{
				LOG_ERR << "Failed to backup data";
			}

			g_condVarEnd.notify_one();
		});
	sigHandler.setAction(SIGTERM, [](int singal)
		{
			if(!(g_vtClient && g_vtClient->saveDataToJson()))
			{
				LOG_ERR << "Failed to backup data";
			}

			g_condVarEnd.notify_one();
		});

	try
	{
		std::fstream inputStream{ "GPSData.txt" };// SHOULD LINK DIRECTLY TO GPS OUTPUTSTREAM BUT FOR NOW THIS SHOULD DO

		std::string configPath{ "VTConfig.json"};

		g_vtClient = std::make_unique<model::VehicleTrackerClient>(configPath, inputStream);
		// should be taken from config file

		std::mutex mutexEnd;
		std::unique_lock<std::mutex> ulock(mutexEnd);
		g_condVarEnd.wait(ulock);
	}
	catch (std::runtime_error& err)
	{
		LOG_ERR << "Client failed to init" << err.what();
	}
}