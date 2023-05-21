#pragma once
#ifndef CARDETECT_CVISION_CAMERA_HPP
#define CARDETECT_CVISION_CAMERA_HPP

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <shared_mutex>

#include <boost/optional.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "utile/Logger.hpp"

namespace cvision
{
	using namespace common::utile;
	class Camera
	{
	private:
		std::shared_ptr<cv::VideoCapture> videoCapture_;
		int16_t id_;
		std::string pathVideo_;
		std::thread threadRead_;
		std::mutex mutexRead_;
		std::shared_mutex mutexRun_;
		std::condition_variable condVarRead_;
		boost::optional<cv::Mat> currentImage_;
		
		LOGGER("Camera");
	public:
		Camera(const int16_t id);
		Camera(const std::string pathVideo);
		Camera(const Camera&) = delete;
		virtual ~Camera();

		bool start();
		void stop();
		bool isRunning();

		uint16_t getId() const;
		boost::optional<cv::Mat> getImage();
	};
} // namespace cvision
#endif // #CARDETECT_CVISION_CAMERA_HPP
