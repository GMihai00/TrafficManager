#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "ImageRender.hpp"

namespace cvision
{
	void ImageRender::loadImage(const cv::Mat& image)
	{
		imageQueue_.push(image);
		condVarRender_.notify_one();
	}

	bool ImageRender::startRendering()
	{
		if (isRunning_) { return false; }

		isRunning_ = true;
		threadRender_ = std::thread([this]()
			{
				while (!shuttingDown_)
				{
					std::unique_lock<std::mutex> ulock(mutexRender_);
					if (imageQueue_.empty() && !shuttingDown_)
						condVarRender_.wait(ulock, [this] { return !imageQueue_.empty() || shuttingDown_; });

					if (shuttingDown_)
						continue;

					cv::Mat img = imageQueue_.front();
					cv::imshow("FRAME", img);
					cv::waitKey(1);
					imageQueue_.pop();
				}
			});
		return true;
	}

	void ImageRender::stopRendering()
	{
		shuttingDown_ = true;
		condVarRender_.notify_one();
		if (threadRender_.joinable())
			threadRender_.join();
	}

	bool ImageRender::isRunning() const
	{
		return isRunning_;
	}

	ImageRender::~ImageRender()
	{
		stopRendering();
	}
} // namespace cvision