#ifndef MODEL_COSTUMCLIENT
#define MODEL_COSTUMCLIENT

#include <utility>
#include <condition_variable>

#include "net/Client.hpp"
#include "net/Message.hpp"
#include "MessageTypes.hpp"
#include "utile/MessageIdProvider.hpp"
#include "cvision/ObjectTracker.hpp"
#include "utile/Logger.hpp"
#include "utile/Observer.hpp"

namespace model
{
    // CONFIG TO DETERMINE LANE
    class VehicleTrackerClient : public ipc::net::Client<ipc::VehicleDetectionMessages>
    {
    private:
        ipc::utile::MessageIdProvider<ipc::VehicleDetectionMessages> messageIdProvider_;
        cvision::ObjectTracker carTracker_;
        common::utile::IObserverPtr observer_;

        LOGGER("VEHICLETRAKER-CLIENT");
        bool startTrackingCars();
        void stopTrackingCars();
    public:
        VehicleTrackerClient();
        ~VehicleTrackerClient();

        void handleNewCarData();
        // THIS METHOD SHOULD BE CALLED ONLY WHEN CARCOUNT IS UPDATED
        bool sendData(size_t numberOfCars, bool leaving = false);
    };
} // namespace model
#endif
