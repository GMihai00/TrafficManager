#pragma once
#ifndef CARDETECT_CVISION_CARDETECT_HPP
#define CARDETECT_CVISION_CARDETECT_HPP

#include <queue>
#include "Utils.hpp"
#include "opencv2/objdetect.hpp"
#include "MovingObjectGroup.hpp"

namespace cvision
{
	const std::string trainedDataPath = ".\\resources\\carsTrainedData.xml"; // THIS PATH SEEMS TO GO BAD NEED TO CHECK WHY
	class CarDetect
	{
	private:
		std::queue<MovingObjectGroupPtr> imageQueue_;
		std::thread threadDetect_;
		std::mutex mutexDetect_;
		std::condition_variable condVarDetect_;
		cv::Mat frame_;
		std::atomic<bool> isRunning_ = false;
		cv::CascadeClassifier carCascade_;
		size_t getCarsPresentInImage(const cv::Mat& image);
	public:
		CarDetect();
		~CarDetect();
		void setFrame(const cv::Mat& image);
		void loadObjectGroup(const MovingObjectGroupPtr objectGroup);
		void waitForFinish();
		bool startDetecting();
		bool isRunning() const;
		void stopDetecting();
	};
	} // namespace cvision

#endif // #CARDETECT_CVISION_CARDETECT_HPP