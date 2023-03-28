#pragma once
#ifndef CARDETECT_CVISION_CARDETECT_HPP
#define CARDETECT_CVISION_CARDETECT_HPP

#include <queue>
#include <map>
#include "Utils.hpp"
#include "opencv2/objdetect.hpp"
#include "MovingObjectGroup.hpp"
#include "utile/ThreadSafeQueue.hpp"

namespace cvision
{
	const std::string trainedDataPath = "..\\resources\\carsTrainedData.xml"; // THIS PATH SEEMS TO GO BAD NEED TO CHECK WHY
	class CarDetect
	{
	private:
		common::utile::ThreadSafeQueue<std::pair<uint32_t, cv::Mat>> taskQueue_;
		std::map<uint32_t, uint8_t> carCountTaskMap_;

		std::thread threadDetect_;
		std::mutex mutexDetect_;
		std::condition_variable condVarDetect_;
		std::atomic<bool> shuttingDown_ = false;
		std::atomic<bool> isRunning_ = false;
		cv::CascadeClassifier carCascade_;
		size_t getCarsPresentInImage(const cv::Mat& image);
	public:
		CarDetect();
		~CarDetect();

		void loadTask(const uint32_t id ,const const cv::Mat& image);
		std::map<uint32_t, uint8_t> waitForFinish();
		bool startDetecting();
		bool isRunning() const;
		void stopDetecting();
	};
	} // namespace cvision

#endif // #CARDETECT_CVISION_CARDETECT_HPP