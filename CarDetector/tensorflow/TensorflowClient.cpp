#include "TensorflowClient.hpp"
#include <opencv2/opencv.hpp>

namespace tensorflow
{

	uint8_t TensorflowClient::get_car_count_inside_image(const cv::Mat& image)
	{
		LOG_INF << __FUNCTION__;

        if (!connection_)
        {
            LOG_ERR << "Connection not established, failed to send message";
            return 0;
        }

        ipc::net::Message<TenserflowMessages> message;
        message.header.type = TenserflowMessages::GET_CARS_COUNT;
        message.header.id = messageIdProvider_.provideId(TenserflowMessages::GET_CARS_COUNT);
        message.header.hasPriority = false;

        std::vector<uint8_t> bytes;
        cv::imencode(".jpg", image, bytes);
        
        message << bytes;

        connection_->send(message);
        
        if (!waitForAnswear(5000))
        {
            return 0;
        }

        //asta poate sa dea in double locking to fix later
        auto answear = getLastUnreadAnswear();
        if (!answear.has_value()) { return 0; }

        uint8_t nr_cars = 0;
        answear.value().first.msg >> nr_cars;

        return nr_cars;
	}

} // namespace tensorflow