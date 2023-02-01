#pragma once
#ifndef MODEL_CVISION_OBJECTTRACKER_HPP
#define MODEL_CVISION_OBJECTTRACKER_HPP

// TODO: CHECK IF ALL INCLUDES ARE REQUIRED
#include <conio.h>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>
#include <list>
#include <boost/optional.hpp>
#include <condition_variable>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "utile/ThreadSafeQueue.hpp"
#include "utile/Logger.hpp"
#include "Camera.hpp"
#include "CarDetect.hpp"
#include "MovingObjectGroup.hpp"
#include "Utils.hpp"
#include "ImageProcessor.hpp"
#include "ImageRender.hpp"

namespace cvision
{
	class ObjectTracker
	{
	private:
		common::utile::ThreadSafeQueue<cv::Mat> imageQueue_;
		std::shared_ptr<ImageProcessor> imageProcessor_;
		std::shared_ptr<ImageRender> imageRender_;
		MovingObjGroupPtrList movingObjects_;
		std::shared_ptr<CarDetect> carDetector_;
		MovingObjGroupPtrList cars_;
		boost::optional<cv::Mat> firstImageFrame_;
		boost::optional<cv::Mat> secondImageFrame_;
		int horizontalLinePosition_;
		cv::Point crossingLineRight_[2];
		cv::Point crossingLineLeft_[2];
		size_t carCountLeft_ = 0;
		size_t carCountRight_ = 0;
		std::thread threadProcess_;
		std::thread threadCamera_;
		Camera camera_;
		std::mutex mutexProcess_;
		std::condition_variable condVarProcess_;
		LOGGER("CAR-TRACKER");

		void setupLines();
		void processImages();
		void waitForFirstFrameApperance();

		void addNewMovingObject(const std::shared_ptr<MovingObject>& currentFrameMovingObj);
		void matchFoundObjToExistingOnes(MovingObjPtrList& currentFrameMovingObj);
		std::pair<std::shared_ptr<MovingObjectGroup>, double> getClosestMovingObject(
			const MovingObjGroupPtrList& movingObjGroupList, const std::shared_ptr<MovingObject>& targetMovingObject);
		bool checkIfCarsCrossedRight();
		bool checkIfCarsCrossedLeft();

		void drawObjInfoOnImage(cv::Mat& img);
		void drawObjCountOnImage(cv::Mat& img);
		void drawRezultsOnImage(cv::Mat& img);

	public:
		ObjectTracker(const uint16_t idCamera);
		ObjectTracker(const std::string videoPath);
		ObjectTracker(const ObjectTracker&) = delete;
		virtual ~ObjectTracker() noexcept;
		bool startTracking();
		void stopTracking();
	};
} // namespace cvision
#endif // #MODEL_CVISION_OBJECTTRACKER_HPP