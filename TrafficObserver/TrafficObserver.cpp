#include <mutex>
#include <memory>
#include <condition_variable>

#include "model/TrafficObserverClient.hpp"
#include "utile/SignalHandler.hpp"
#include "utile/Logger.hpp"
#include "utile/CommandLineParser.hpp"
#include "net/Message.hpp"
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

std::optional<std::filesystem::path> getVideoPath(const CommandLineParser& commandLine)
{
	constexpr std::array<std::string_view, 2> optionNames = { "-vp", "--video_path" };

	for (const auto& optionName : optionNames)
	{
		auto option = commandLine.getOption(optionName);
		if (option.has_value())
		{
			auto videoPath = std::filesystem::path(option.value());
			if (!std::filesystem::is_regular_file(videoPath))
			{
				LOG_WARN << "Invalid file path: " << videoPath;
				return {};
			}
			return videoPath;
		}
	}

	return {};
}

bool g_shouldStop = false;

int main(int argc, char* argv[])
{
	SignalHandler sigHandler{};
	sigHandler.setAction(SIGINT, [](int /*singal*/)
		{
			g_shouldStop = true;
		});
	sigHandler.setAction(SIGTERM, [](int /*singal*/)
		{
			g_shouldStop = true;
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

	std::shared_ptr<model::TrafficObserverClient> client;
	auto videoPath = getVideoPath(commandLine);
	if (videoPath.has_value())
	{
		client = std::make_shared<model::TrafficObserverClient>(keyword.value(), videoPath.value());
	}
	else
	{
		client = std::make_shared<model::TrafficObserverClient>(keyword.value());
	}

	if (!client->connect(serverIp.value(), serverPort.value()))
	{
		LOG_ERR << "Failed to connect to " << serverIp.value() << ":" << serverPort.value();
		exit(5);
	}

	// can't be avoided
	while (!g_shouldStop)
	{
		Sleep(500);
	}
 //  /* std::mutex mutexEnd;
 //   std::unique_lock<std::mutex> ulock(mutexEnd);
 //   g_condVarEnd.wait(ulock);*/
 //   return 0;
}