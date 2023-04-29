#pragma once
#ifndef CARDETECT_CVISION_TENSERFLOWCLIENT_HPP
#define CARDETECT_CVISION_TENSERFLOWCLIENT_HPP

#include <opencv2/core/core.hpp>

#include "net/Client.hpp"
#include "net/Message.hpp"
#include "MessageTypes.hpp"
#include "utile/MessageIdProvider.hpp"

#include "utile/Logger.hpp"

namespace tensorflow
{
    enum TenserflowMessages : uint8_t
    {
        GET_CARS_COUNT
    };

    class TensorflowClient : public ipc::net::Client<TenserflowMessages>
    {
    private:
        ipc::utile::MessageIdProvider<TenserflowMessages> messageIdProvider_;
        
        LOGGER("TENSERFLOW-CLIENT");
    public:
        TensorflowClient() = default;
        ~TensorflowClient() noexcept = default;

        uint8_t get_car_count_inside_image(const cv::Mat& image);
    };
} // namespace tensorflow
#endif // #CVISION_TENSERFLOWCLIENT_HPP
