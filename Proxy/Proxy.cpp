#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <condition_variable>

#include "db/Proxy.hpp"
#include "model/ProxyServer.hpp"
#include "utile/CommandLineParser.hpp"
#include "utile/Logger.hpp"
#include "utile/SignalHandler.hpp"
#include "utile/ErrorCodes.hpp"
#include "utile/ConfigHelpers.hpp"

using namespace common::utile;
using namespace common;

LOGGER("MAIN");

std::optional<std::filesystem::path> getProxyConfigFile(const CommandLineParser& commandLine)
{
	constexpr std::array<std::string_view, 2> optionNames = { "-p_conf", "--proxy_config" };

	for (const auto& optionName : optionNames)
	{
		auto option = commandLine.getOption(optionName);
		if (option.has_value())
		{
			std::error_code ec;
			auto file = std::filesystem::path(std::string(option.value()));
			if (std::filesystem::is_regular_file(file, ec))
			{
				return file;
			}
			return {};
		}
	}

	return {};
}

bool g_shouldStop = false;

int main(int argc, char* argv[])
{
	SignalHandler sigHandler{};
	auto commandLine = CommandLineParser(argc, argv);
	
	auto proxyConfigFile = getProxyConfigFile(commandLine);
	if (!proxyConfigFile.has_value())
	{
		LOG_ERR << "Failed to get Proxy config file";
		exit(5);
	}

	auto maybeConfig = common::utile::loadProxyConfig(proxyConfigFile.value().string());

	if (!maybeConfig.has_value())
	{
		LOG_ERR << "Invalid proxy config file";
		exit(5);
	}

	LOG_DBG << maybeConfig->toString();
	try
	{
		::model::ProxyServer proxySERVER(maybeConfig.value());
		if (!proxySERVER.start())
		{
			LOG_ERR << "Failed to start server";
			return ipc::utile::SERVER_FAILURE;
		}

		sigHandler.setAction(SIGINT, [](int /*singal*/) { g_shouldStop = true; });
		sigHandler.setAction(SIGTERM, [](int /*singal*/) { g_shouldStop = true; });

		while (!g_shouldStop)
		{
			Sleep(500);
		}
		LOG_INF << "Finished waiting";
	}
	catch (const std::exception& err)
	{
		LOG_ERR << "Failed to start server: " << err.what();
		return ipc::utile::SERVER_FAILURE;
	}

	return 0;
}