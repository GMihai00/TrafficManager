#pragma once
#ifndef MODEL_CVISION_UTILS_HPP
#define MODEL_CVISION_UTILS_HPP

#include <list>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define SCALAR_BLACK cv::Scalar(0.0, 0.0, 0.0)
#define SCALAR_PINK cv::Scalar(255.0, 102.0, 255.0)
#define SCALAR_YELLOW cv::Scalar(0.0, 255.0, 255.0)
#define SCALAR_GREEN cv::Scalar(0.0, 200.0, 0.0)
#define SCALAR_RED cv::Scalar(0.0, 0.0, 255.0)

namespace cvision
{
	typedef std::vector<std::vector<cv::Point> > PointMatrix;
	double distanceBetweenPoints(cv::Point point1, cv::Point point2);

} // namespace cvision
#endif // #MODEL_CVISION_UTILS_HPP