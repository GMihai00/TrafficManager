#include <iostream>
#include "CarDetect.hpp"

namespace cvision
{
	CarDetect::CarDetect()
	{
		if (!carCascade_.load(trainedDataPath))
		{
			throw std::runtime_error("INVALID TRAINING DATA FILE");
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

	size_t CarDetect::getCarsPresentInImage(const cv::Mat& image)
	{
		//return 1u; 
		// FOR NOW CAR DETECTION IS BEEING DISABLED UNTIL A BETTER DATASET WILL BE CREATED
		std::vector<cv::Rect> cars;
		carCascade_.detectMultiScale(image, cars);
			
		return cars.size();
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
						auto nrCars = getCarsPresentInImage(task.value().second);
						carCountTaskMap_[task.value().first] = nrCars;
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
		auto rez = carCountTaskMap_;
		carCountTaskMap_.clear();

		return rez;
	}
} // namespace cvision