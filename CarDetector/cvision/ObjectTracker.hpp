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
#include <optional>
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
#include "utile/Observer.hpp"

namespace cvision
{
	class ObjectTracker
	{
	private:
		uint32_t taskId_ = 0;
		common::utile::ThreadSafeQueue<cv::Mat> imageQueue_;
		std::shared_ptr<ImageProcessor> imageProcessor_;
		std::shared_ptr<ImageRender> imageRender_;
		MovingObjGroupPtrList movingObjects_;
		std::shared_ptr<CarDetect> carDetector_;
		MovingObjGroupPtrList cars_;
		std::map<uint32_t, MovingObjectGroupPtr> taskIdToObjGroup_;
		std::optional<cv::Mat> firstImageFrame_;
		std::optional<cv::Mat> secondImageFrame_;
		int horizontalLinePosition_;
		cv::Point crossingLineRight_[2];
		cv::Point crossingLineLeft_[2];
		size_t carCountLeft_ = 0;
		size_t carCountRight_ = 0;
		size_t carCountLeftDisplay_ = 0;
		size_t carCountRightDisplay_ = 0;

		std::thread threadProcess_;
		std::thread threadCamera_;
		Camera camera_;
		std::mutex mutexProcess_;
		std::condition_variable condVarProcess_;
		bool shouldRender_ = false;

		common::utile::IObserverPtr attachedObserver_;
		LOGGER("CAR-TRACKER");

		void setupLines();
		void processImages();
		void waitForFirstFrameApperance();

		void addNewMovingObject(const MovingObjectPtr& currentFrameMovingObj);
		void matchFoundObjToExistingOnes(MovingObjPtrList& currentFrameMovingObj);
		std::pair<MovingObjectGroupPtr, double> getClosestMovingObject(
			const MovingObjGroupPtrList& movingObjGroupList, const MovingObjectPtr& targetMovingObject);
		bool checkIfCarsCrossedRight();
		bool checkIfCarsCrossedLeft();

		void drawObjInfoOnImage(cv::Mat& img);
		void drawObjCountOnImage(cv::Mat& img);
		void drawRezultsOnImage(cv::Mat& img);
		void detectCarsInOjectGroup(const MovingObjectGroupPtr& objGroup);
	public:
		ObjectTracker(const uint16_t idCamera, IObserverPtr observer = nullptr);
		ObjectTracker(const std::string videoPath, IObserverPtr observer = nullptr);
		ObjectTracker(const ObjectTracker&) = delete;
		virtual ~ObjectTracker() noexcept;
		bool startTracking(bool shouldRender = false);
		void stopTracking();

		std::pair<size_t, size_t> getCarCount();
		void resetCarCount();
		void subscribe(IObserverPtr observer);
	};
} // namespace cvision
#endif // #MODEL_CVISION_OBJECTTRACKER_HPP