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

	db::ProxyPtr dbProxy = std::make_shared<db::Proxy>(host.value(), port.value(), 0, coveredArea);

	try
	{
		model::ProxyServer proxySERVER(host.value(), port.value(), dbProxy);
		if (!proxySERVER.start())
		{
			LOG_ERR << "Failed to start server";
		}

		sigHandler.setAction(SIGINT, [](int singal) { g_condVarEnd.notify_one(); });
		std::mutex mutexEnd;
		std::unique_lock<std::mutex> ulock(mutexEnd);
		g_condVarEnd.wait(ulock);
		LOG_INF << "Finished waiting";
		// somehow destructors are failing don't know how
	}
	catch (std::runtime_error& err)
	{
		LOG_ERR << err.what();
		exit(10);
	}
	return 0;
}