#pragma once
#ifndef MODEL_CVISION_IMAGERENDER_HPP
#define MODEL_CVISION_IMAGERENDER_HPP

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "Utils.hpp"

namespace cvision
{
	class ImageRender
	{
	private:
		std::queue<cv::Mat> imageQueue_;
		std::thread threadRender_;
		std::mutex mutexRender_;
		std::condition_variable condVarRender_;
		std::condition_variable condVarUpdate_;
		std::atomic<bool> isRunning_ = false;
		std::atomic<bool> shuttingDown_ = false;
	public:
		ImageRender() = default;
		ImageRender(const ImageRender&) = delete;
		~ImageRender();

		void loadImage(const cv::Mat& image);
		bool startRendering();
		bool isRunning() const;
		void stopRendering();
	};
} // namespace cvision
#endif // #MODEL_CVISION_IMAGERENDER_HPP