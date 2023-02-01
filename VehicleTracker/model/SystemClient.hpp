#ifndef MODEL_COSTUMCLIENT
#define MODEL_COSTUMCLIENT

#include <utility>

#include "net/Client.hpp"
#include "net/Message.hpp"
#include "MessageTypes.hpp"
#include "utile/MessageIdProvider.hpp"
#include "cvision/ObjectTracker.hpp";
#include "utile/Logger.hpp"

namespace model
{

    class SystemClient : public ipc::net::Client<ipc::VehicleDetectionMessages>
    {
    private:
        ipc::utile::MessageIdProvider<ipc::VehicleDetectionMessages> messageIdProvider_;
        cvision::ObjectTracker carTracker_;

        bool startTrackingCars();
        void stopTrackingCars();
    public:
        SystemClient();
        ~SystemClient();
        bool sendVehicleData(int numberOfCars, bool hasPriority, bool rightLane);
    };
} // namespace model
#endif
