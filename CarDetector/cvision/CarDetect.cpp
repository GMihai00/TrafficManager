#include <iostream>
#include "CarDetect.hpp"

namespace cvision
{
	CarDetect::CarDetect()
	{
		if (!tensorflowClient_.connect(TENSORFLOW_SERVER_HOST, TENSORFLOW_SERVER_PORT))
		{
			throw std::runtime_error("Failed to connect to tensorflow server");
		}

	}

	CarDetect::~CarDetect()
	{
		stopDetecting();
	}

	void CarDetect::loadTask(const uint32_t id, const const cv::Mat& image)
	{
		std::pair<uint32_t, cv::Mat> task = { id, image };
		taskQueue_.push(task);
		condVarDetect_.notify_one();
	}

	std::future<uint8_t> CarDetect::getCarsPresentInImage(const cv::Mat& image)
	{
		return std::async([this, image]() { 
			return tensorflowClient_.get_car_count_inside_image(image);
		});
	}

	bool CarDetect::startDetecting()
	{
		if (isRunning_)
		{
			return false;
		}
		isRunning_ = true;
		threadDetect_ = std::thread([this]()
			{
				while (!shuttingDown_)
				{
					std::unique_lock<std::mutex> ulock(mutexDetect_);
					condVarDetect_.wait(ulock, [this] {	return !taskQueue_.empty() || shuttingDown_; });

					if (shuttingDown_)
						continue;

					auto task = taskQueue_.pop();
					if (task.has_value())
					{
						carCountTaskMap_[task.value().first] = getCarsPresentInImage(task.value().second);
					}
					
					ulock.unlock();
				}
			});
		return true;
	}

	bool CarDetect::isRunning() const
	{
		return isRunning_;
	}

	void CarDetect::stopDetecting()
	{
		shuttingDown_ = true;
		condVarDetect_.notify_one();
		if (threadDetect_.joinable())
			threadDetect_.join();
	}

	std::map<uint32_t, uint8_t> CarDetect::waitForFinish()
	{
		while (!taskQueue_.empty())
		{
			condVarDetect_.notify_one();
			std::this_thread::sleep_for(std::chrono::microseconds(10));
			// do nothing remove busy wait somehow
		}

		std::map<uint32_t, uint8_t> rez;
		for (auto& [key, future] : carCountTaskMap_) {

			uint8_t value = future.get();
			rez.insert(std::make_pair(key, value));
		}
		carCountTaskMap_.clear();

		return rez;
	}
} // namespace cvision