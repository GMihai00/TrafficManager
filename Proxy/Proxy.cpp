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

using namespace common::utile;
using namespace common;

LOGGER("MAIN");

std::optional<ipc::utile::IP_ADRESS> getHost(const CommandLineParser& commandLine)
{
	constexpr std::array<std::string_view, 2> optionNames = { "-h", "--host" };

	for (const auto& optionName : optionNames)
	{
		auto option = commandLine.getOption(optionName);
		if (option.has_value())
			return std::string(option.value());
	}

	return {};
}

std::optional<ipc::utile::PORT> getPort(const CommandLineParser& commandLine)
{
	constexpr std::array<std::string_view, 2> optionNames = { "-p", "--port" };

	for (const auto& optionName : optionNames)
	{
		auto option = commandLine.getOption(optionName);
		if (option.has_value())
			return std::stoi(std::string(option.value()));
	}

	return {};

}

namespace details
{
	std::optional<GeoCoordinate<DecimalCoordinate>> 
		getCoordinate(const CommandLineParser& commandLine, const std::array<std::string_view, 2>& optionNames)
	{
		for (const auto& optionName : optionNames)
		{
			auto option = commandLine.getOption(optionName);
			if (option.has_value())
			{
				try
				{
					auto coordinate = GeoCoordinate<DecimalCoordinate>(std::string(option.value()));
					return coordinate;
				}
				catch (std::runtime_error err)
				{
					LOG_ERR << err.what() << " Please provide <lat>,<lon> format data";
					return {};
				}
			}
		}
		return {};
	}

	std::optional<GeoCoordinate<DecimalCoordinate>> getSWBound(const CommandLineParser& commandLine)
	{
		constexpr std::array<std::string_view, 2> optionNames = { "-bsw", "--bound_sw" };
		return getCoordinate(commandLine, optionNames);
	}

	std::optional<GeoCoordinate<DecimalCoordinate>> getNEBound(const CommandLineParser& commandLine)
	{
		constexpr std::array<std::string_view, 2> optionNames = { "-bne", "--bound_ne" };
		return getCoordinate(commandLine, optionNames);
	}
}

db::BoundingRectPtr getCoveredArea(const CommandLineParser& commandLine)
{
	auto boundSW = details::getSWBound(commandLine);
	if (!boundSW.has_value())
	{
		LOG_ERR << "SW bound not found";
		return nullptr;
	}
	auto boundNE = details::getNEBound(commandLine);
	if (!boundNE.has_value())
	{
		LOG_ERR << "NE bound not found";
		return nullptr;
	}

	return std::make_shared<db::BoundingRect>(boundSW.value(), boundNE.value());
}

std::optional<std::string> getDBServer(const CommandLineParser& commandLine)
{
	constexpr std::array<std::string_view, 2> optionNames = { "-s", "--server" };

	for (const auto& optionName : optionNames)
	{
		auto option = commandLine.getOption(optionName);
		if (option.has_value())
			return std::string(option.value());
	}

	return {};
}

std::optional<std::string> getDBUsername(const CommandLineParser& commandLine)
{
	constexpr std::array<std::string_view, 2> optionNames = { "-u", "--username" };

	for (const auto& optionName : optionNames)
	{
		auto option = commandLine.getOption(optionName);
		if (option.has_value())
			return std::string(option.value());
	}

	return {};
}

std::optional<std::string> getDBPassword(const CommandLineParser& commandLine)
{
	constexpr std::array<std::string_view, 2> optionNames = { "-ps", "--password" };

	for (const auto& optionName : optionNames)
	{
		auto option = commandLine.getOption(optionName);
		if (option.has_value())
			return std::string(option.value());
	}

	return {};
}

// NEEDS TO BE MADE GLOBAL DUE TO BEEING UNABLE TO CAPTURE VARIABLES INSIDE LAMBDAS PASSED TO SIGNALHANDLER
std::condition_variable g_condVarEnd;

int main(int argc, char* argv[])
{
	SignalHandler sigHandler{};
	auto commandLine = CommandLineParser(argc, argv);
	
	auto host = getHost(commandLine);
	if (!host.has_value())
	{
		LOG_ERR << "Host addres not defined";
		exit(5);
	}

	auto port = getPort(commandLine);
	if (!port.has_value())
	{
		LOG_ERR << "Port not defined";
		exit(5);
	}

	auto coveredArea = getCoveredArea(commandLine);
	if (!coveredArea)
	{
		LOG_ERR << "Covered area not defined";
		exit(5);
	}

	// "tcp://localhost:3306/traffic_manager";
	auto bdServer = getDBServer(commandLine);
	if (!bdServer.has_value())
	{
		LOG_ERR << "Proxy DB Server not defined";
		exit(5);
	}

	// "root"
	auto bdUsername = getDBUsername(commandLine);
	// "everyday password"
	auto bdPassword = getDBPassword(commandLine);
	if (!bdUsername.has_value() || !bdPassword.has_value())
	{
		LOG_ERR << "DB credentials missing";
		exit(5);
	}

	db::ProxyPtr dbProxy = std::make_shared<db::Proxy>(host.value(), port.value(), 0, coveredArea);

	try
	{
		model::ProxyServer proxySERVER(host.value(), port.value(), dbProxy, ::utile::DBConnectionData(bdServer.value(), bdUsername.value(), bdPassword.value()));
		if (!proxySERVER.start())
		{
			LOG_ERR << "Failed to start server";
			return ipc::utile::SERVER_FAILURE;
		}

		sigHandler.setAction(SIGINT, [](int /*singal*/) { g_condVarEnd.notify_one(); });
		std::mutex mutexEnd;
		std::unique_lock<std::mutex> ulock(mutexEnd);
		g_condVarEnd.wait(ulock);
		LOG_INF << "Finished waiting";
	}
	catch (const std::exception& err)
	{
		LOG_ERR << "Failed to start server: " << err.what();
		return ipc::utile::SERVER_FAILURE;
	}

	return 0;
}