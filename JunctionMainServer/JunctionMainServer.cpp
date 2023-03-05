#include <condition_variable>
#include <mutex>

#include "model/JunctionServer.hpp"
#include "utile/ConfigLoader.hpp"
#include "utile/SignalHandler.hpp"
#include "utile/ErrorCodes.hpp"

using namespace common::utile;

LOGGER("MAIN");

std::condition_variable g_condVarEnd;

int main()
{
	SignalHandler sigHandler{};

	sigHandler.setAction(SIGINT, [](int singal)
		{

			g_condVarEnd.notify_one();
		});
	sigHandler.setAction(SIGTERM, [](int singal)
		{

			g_condVarEnd.notify_one();
		});

	utile::ConfigLoader configLoader;
	auto maybeConfig = configLoader.load("C:\\Users\\Mihai Gherghinescu\\source\\repos\\TrafficManager\\resources\\JunctionConfig.json"); // to change to relative path

	if (!maybeConfig.has_value())
	{
		LOG_ERR << "Failed to load config file";
		exit(5);
	}

	try
	{
		model::JunctionServer junctionServer(maybeConfig.value());

		if (!junctionServer.start())
		{
			LOG_ERR << "Failed to start server";
			return ipc::utile::SERVER_FAILURE;
		}

		std::mutex mutexEnd;
		std::unique_lock<std::mutex> ulock(mutexEnd);
		g_condVarEnd.wait(ulock);
	}
	catch (const std::exception& err)
	{
		LOG_ERR << "Failed to start server" << err.what();
		return ipc::utile::SERVER_FAILURE;
	}
}