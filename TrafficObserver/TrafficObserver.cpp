#include <mutex>
#include <condition_variable>

#include "model/TrafficObserverClient.hpp"
#include "utile/SignalHandler.hpp"
#include "utile/Logger.hpp"
#include "utile/CommandLineParser.hpp"

using namespace common::utile;

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


std::optional<std::string> getKeyword(const CommandLineParser& commandLine)
{
	constexpr std::array<std::string_view, 2> optionNames = { "-k", "--keyword" };

	for (const auto& optionName : optionNames)
	{
		auto option = commandLine.getOption(optionName);
		if (option.has_value())
			return std::string(option.value());
	}

	return {};

}

bool usingRightLane(const CommandLineParser& commandLine)
{
	constexpr std::array<std::string_view, 2> optionNames = { "-rl", "--rightlane" };
	for (const auto& optionName : optionNames)
	{
		auto option = commandLine.getOption(optionName);
		if (option.has_value())
			return std::stoi(std::string(option.value()));
	}
	return false;
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
	auto serverIp = getHost(commandLine);
	auto serverPort = getPort(commandLine);
	
	if (!serverIp.has_value() || !serverPort.has_value())
	{
		LOG_ERR << "Server adress missing";
		exit(5);
	}

	auto keyword = getKeyword(commandLine);
	if (!keyword.has_value())
	{
		LOG_ERR << "Keyword missing";
		exit(5);
	}

    model::TrafficObserverClient client(keyword.value(), usingRightLane(commandLine));
    client.connect(serverIp.value(), serverPort.value());

    std::mutex mutexEnd;
    std::unique_lock<std::mutex> ulock(mutexEnd);
    g_condVarEnd.wait(ulock);
    return 0;
}