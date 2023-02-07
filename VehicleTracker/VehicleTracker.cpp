#include "model/VehicleTrackerClient.hpp"

// TO MOVE THIS THINGS MAYBE
constexpr auto TCP_IP = "127.0.0.1";
constexpr auto TCP_PORT = 6000;
constexpr auto BUFFER_SIZE = 1024;

int main()
{
    model::VehicleTrackerClient client;
    client.connect(TCP_IP, TCP_PORT);
    while (true)
    {

    }
    return 0;
}