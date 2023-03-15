#include <condition_variable>
#include <mutex>
#include <filesystem>

#include "model/JunctionServer.hpp"
#include "utile/ConfigHelpers.hpp"
#include "utile/SignalHandler.hpp"
#include "utile/ErrorCodes.hpp"
#include "utile/CommandLineParser.hpp"
#include "utile/TypeConverters.hpp"

using namespace common::utile;

LOGGER("MAIN");

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

std::condition_variable g_condVarEnd;

int main(int argc, char* argv[])
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

	auto commandLine = CommandLineParser(argc, argv);

	// "C:\\Users\\Mihai Gherghinescu\\source\\repos\\TrafficManager\\resources\\JunctionConfig.json"
	auto maybeConfigFile = getConfigFile(commandLine);
	if (!maybeConfigFile.has_value())
	{
		LOG_ERR << "Config file not specified";
		exit(5);
	}

	auto maybeConfig = common::utile::loadJMSConfig(maybeConfigFile.value());

	if (!maybeConfig.has_value())
	{
		LOG_ERR << "Failed to load config file";
		exit(5);
	}

	try
	{
		::model::JunctionServer junctionServer(maybeConfig.value());

		if (!junctionServer.start())
		{
			LOG_ERR << "Failed to start server";
			return ipc::utile::SERVER_FAILURE;
		}

		LOG_INF << "Server started";
		std::mutex mutexEnd;
		std::unique_lock<std::mutex> ulock(mutexEnd);
		g_condVarEnd.wait(ulock);
	}
	catch (const std::exception& err)
	{
		LOG_ERR << "Failed to start server" << common::utile::utf16_to_utf8(err.what());
		return ipc::utile::SERVER_FAILURE;
	}
}