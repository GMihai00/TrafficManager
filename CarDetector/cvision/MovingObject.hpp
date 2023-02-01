#pragma once
#ifndef MODEL_CVISION_MOVINGOBJECT_HPP
#define MODEL_CVISION_MOVINGOBJECT_HPP

#include <vector>
#include <memory>

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

namespace cvision
{
	class MovingObject {
	private:
		std::vector<cv::Point> contour_;
		cv::Rect boundingRect_;
	public:
		MovingObject() = delete;
		MovingObject(const std::vector<cv::Point>& contour);
		MovingObject(const MovingObject& cpy);
		~MovingObject() = default;

		double getDiagonalSize() const;
		double getAspectRatio() const;
		double getWidth() const;
		double getHeight() const;
		double getArea() const;
		double getContourArea() const;
		cv::Point getCenter() const;
		std::vector<cv::Point> getContour() const;
		cv::Rect getBoudingRect() const;
	};
	typedef std::shared_ptr<MovingObject> MovingObjectPtr;
	typedef std::list<MovingObjectPtr> MovingObjPtrList;
} // namespace cvision
#endif // #MODEL_CVISION_MOVINGOBJECT_HPP