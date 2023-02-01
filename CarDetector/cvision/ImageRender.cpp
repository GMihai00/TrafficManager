#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "ImageRender.hpp"

namespace cvision
{
	void ImageRender::loadImage(const cv::Mat& image)
	{
		std::scoped_lock lock(mutexRender_);
		imageQueue_.push(image);
		condVarRender_.notify_one();
	}

	bool ImageRender::startRendering()
	{
		if (isRunning_) { return false; }

		isRunning_ = true;
		threadRender_ = std::thread([this]()
			{
				while (true)
				{
					std::unique_lock<std::mutex> ulock(mutexRender_);
					condVarRender_.wait(ulock, [this] { return !imageQueue_.empty(); });

					auto img = imageQueue_.pop();
					if (img)
					{
						cv::imshow("FRAME", img.value());
						cv::waitKey(1);
					}
				}
			});
		return true;
	}

	void ImageRender::stopRendering()
	{
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