#include "Camera.hpp"

#include <string>
#include <exception>
#include <typeinfo>
#include <stdexcept>


namespace cvision
{
	Camera::Camera(const int16_t id) :
		id_{ id }
	{
		LOG_SET_NAME("CAMERA-" + std::to_string(id));
	}

	Camera::Camera(const std::string pathVideo) :
		pathVideo_(pathVideo),
		id_(-1),
		logger_("Camera-" + pathVideo)
	{
	}

	Camera::~Camera()
	{
		stop();
	}

	bool Camera::start()
	{
		if (videoCapture_)
		{
			LOG_DBG << "Camera already powered on. Nothing to do";
			return false;
		}
		if (pathVideo_ == "")
		{
			videoCapture_ = std::make_shared<cv::VideoCapture>(id_);
		}
		else
		{
			videoCapture_ = std::make_shared<cv::VideoCapture>(pathVideo_);
		}

		threadRead_ = std::thread([this]()
			{
				while (videoCapture_ != nullptr)
				{
					std::unique_lock<std::mutex> ulock(mutexRead_);
					condVarRead_.wait(ulock, [&] { return currentImage_ == boost::none; });

					cv::Mat img;
					if (videoCapture_ && videoCapture_->read(img))
					{
						cv::resize(img, img, cv::Size(854, 480));
						currentImage_ = img;
					}
					else
					{
						LOG_DBG << "Failed to read image, BUT WHY?";
						videoCapture_ = nullptr;
					}
					ulock.unlock();
				}
			});
		LOG_DBG << "Camera started";
		return true;
	}

	void Camera::stop()
	{
		videoCapture_.reset();
		if (threadRead_.joinable())
			threadRead_.join();
		LOG_DBG << "Camera stopped";
	}

	bool Camera::isRunning()
	{
		std::scoped_lock lock(mutexRead_);
		return videoCapture_ != nullptr;
	}

	uint16_t Camera::getId() const
	{
		return this->id_;
	}

	boost::optional<cv::Mat> Camera::getImage()
	{
		std::scoped_lock lock(mutexRead_);
		if (this->currentImage_ == boost::none)
		{
			condVarRead_.notify_one();
			return boost::none;
		}
		
		boost::optional<cv::Mat> cpy = this->currentImage_.get().clone();
		currentImage_ = boost::none;
		condVarRead_.notify_one();
		return cpy;
	}
} // namespace cvision