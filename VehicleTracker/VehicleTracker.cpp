#include "model/VehicleTrackerClient.hpp"

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <memory>

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
			if (g_vtClient)
				g_vtClient->saveDataToJson();

			g_condVarEnd.notify_one();
		});
	sigHandler.setAction(SIGTERM, [](int singal)
		{
			if (g_vtClient)
			g_vtClient->saveDataToJson();

	g_condVarEnd.notify_one();
		});

	try
	{
		std::fstream inputStream{ "GPSData.txt" };// SHOULD LINK DIRECTLY TO GPS OUTPUTSTREAM BUT FOR NOW THIS SHOULD DO

		std::stack<std::pair<ipc::utile::IP_ADRESS, ipc::utile::PORT>> lastVisitedProxys; // taken from config file
		g_vtClient = std::make_unique<model::VehicleTrackerClient>(inputStream, lastVisitedProxys);
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