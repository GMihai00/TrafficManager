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

	void CarDetect::loadObjectGroup(const MovingObjectGroupPtr objectGroup)
	{
		imageQueue_.push(objectGroup);
		condVarDetect_.notify_one();
	}

	void CarDetect::setFrame(const cv::Mat& image)
	{
		frame_ = image;
	}

	size_t CarDetect::getCarsPresentInImage(const cv::Mat& image)
	{
		return 1u; 
		// FOR NOW CAR DETECTION IS BEEING DISABLED UNTIL A BETTER DATASET WILL BE CREATED
		/*std::vector<cv::Rect> cars;
		carCascade_.detectMultiScale(image, cars);
			
		return cars.size();*/
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
				while (true)
				{
					std::unique_lock<std::mutex> ulock(mutexDetect_);
					condVarDetect_.wait(ulock, [this] {	return !imageQueue_.empty(); });

					std::shared_ptr<MovingObjectGroup> movingObjectGroup = imageQueue_.front();
					imageQueue_.pop();
					if (movingObjectGroup && movingObjectGroup->nrCars() == 0)
					{
						movingObjectGroup->updateCarState(
							getCarsPresentInImage(movingObjectGroup->getCroppedImage(frame_.clone())));
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
		if (threadDetect_.joinable())
			threadDetect_.join();
	}

	void CarDetect::waitForFinish()
	{
		while (!imageQueue_.empty())
		{
			condVarDetect_.notify_one();
			std::this_thread::sleep_for(std::chrono::microseconds(10));
			// do nothing remove busy wait somehow
		}
	}
} // namespace cvision