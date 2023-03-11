#include <mutex>
#include <condition_variable>

#include "model/TrafficObserverClient.hpp"
#include "utile/SignalHandler.hpp"

using namespace common::utile;

constexpr auto TCP_IP = "127.0.0.1";
constexpr auto TCP_PORT = 5000;
constexpr auto BUFFER_SIZE = 1024;

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


    model::TrafficObserverClient client;
    client.connect(TCP_IP, TCP_PORT);

    std::mutex mutexEnd;
    std::unique_lock<std::mutex> ulock(mutexEnd);
    g_condVarEnd.wait(ulock);
    return 0;
}