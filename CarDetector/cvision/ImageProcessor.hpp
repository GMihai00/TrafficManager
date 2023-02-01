#pragma once
#ifndef CARDETECT_CVISION_IMAGEPROCESSOR_HPP
#define CARDETECT_CVISION_IMAGEPROCESSOR_HPP

#include "MovingObject.hpp"
#include "Utils.hpp"

namespace cvision
{
	class ImageProcessor
	{
	public:
		ImageProcessor() = default;
		~ImageProcessor() = default;
		cv::Mat preprocessImage(const cv::Mat& image) const;
		cv::Mat getProcessedMergedImage(const cv::Mat& img1, const cv::Mat& img2) const;
		PointMatrix getImgConvexHulls(cv::Mat& img);
		MovingObjPtrList getMovingObjectsFromImg(cv::Mat& img);
		bool isMinimumObjSize(const MovingObjectPtr& obj);
	};
} // namespace cvision
#endif // #MODEL_CVISION_IMAGEPROCESSOR_HPP