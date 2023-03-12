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
#include "utile/ErrorCodes.hpp"

using namespace common::utile;
using namespace common;

LOGGER("MAIN");

std::optional<std::string> getGpsInputFile(const CommandLineParser& commandLine)
{
	constexpr std::array<std::string_view, 2> optionNames = { "-gps_f", "--gps_file" };

	for (const auto& optionName : optionNames)
	{
		auto option = commandLine.getOption(optionName);
		if (option.has_value())
		{
			std::error_code ec;
			auto file = std::filesystem::path(std::string(option.value()));
			if (std::filesystem::is_regular_file(file, ec))
			{
				return file.string();
			}
			return {};
		}
	}

	return {};
}

std::optional<std::string> getConfigFile(const CommandLineParser& commandLine)
{
	constexpr std::array<std::string_view, 2> optionNames = { "-conf", "--config_file" };

	for (const auto& optionName : optionNames)
	{
		auto option = commandLine.getOption(optionName);
		if (option.has_value())
		{
			std::error_code ec;
			auto file = std::filesystem::path(std::string(option.value()));
			if (std::filesystem::is_regular_file(file, ec))
			{
				return file.string();
			}
			return {};
		}
	}

	return {};
}

// Need to make it global due to signal handling 
std::unique_ptr<model::VehicleTrackerClient> g_vtClient;
std::condition_variable g_condVarEnd;

int main(int argc, char* argv[])
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

	auto commandLine = CommandLineParser(argc, argv);
	auto maybeGpsInputFile = getGpsInputFile(commandLine);
	if (!maybeGpsInputFile.has_value())
	{
		LOG_ERR << "GPS Input missing";
		exit(5);
	}

	auto maybeConfigFile = getConfigFile(commandLine);
	if (!maybeConfigFile.has_value())
	{
		LOG_WARN << "Config file missing";
		maybeConfigFile = "";
	}

	try
	{
		// SHOULD LINK DIRECTLY TO GPS OUTPUTSTREAM BUT FOR NOW THIS SHOULD DO
		std::fstream inputStream{ maybeConfigFile.value()};

		g_vtClient = std::make_unique<model::VehicleTrackerClient>(maybeConfigFile.value(), inputStream);

		g_vtClient->start();
		std::mutex mutexEnd;
		std::unique_lock<std::mutex> ulock(mutexEnd);
		g_condVarEnd.wait(ulock);
	}
	catch (std::runtime_error& err)
	{
		LOG_ERR << "Client failed to init" << err.what();
		return ipc::utile::CLIENT_FAILURE;
	}
	return 0;
}