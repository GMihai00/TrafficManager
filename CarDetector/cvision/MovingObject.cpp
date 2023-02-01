#include "MovingObject.hpp"

namespace cvision
{
	MovingObject::MovingObject(const std::vector<cv::Point>& contour) :
        contour_{ contour },
        boundingRect_{ cv::boundingRect(contour_) }
	{
	}

	MovingObject::MovingObject(const MovingObject& cpy) : 
		contour_ { cpy.contour_ },
		boundingRect_ { cpy.boundingRect_ }
	{

	}

	cv::Point MovingObject::getCenter() const
	{
		cv::Point center;
		center.x = (boundingRect_.x + boundingRect_.x + boundingRect_.width) / 2;
		center.y = (boundingRect_.y + boundingRect_.y + boundingRect_.height) / 2;
			
		return center;
	}

	double MovingObject::getDiagonalSize() const
	{
		return sqrt(pow(boundingRect_.width, 2) + pow(boundingRect_.height, 2));
	}

	double MovingObject::getAspectRatio() const
	{
		return (float)boundingRect_.width / (float)boundingRect_.height;
	}

	double MovingObject::getArea() const
	{
		return this->boundingRect_.area();
	}
        
	double MovingObject::getContourArea() const
	{
		return cv::contourArea(this->contour_) / this->getArea();
	}

	double MovingObject::getWidth() const
	{
		return this->boundingRect_.width;
	}

	double MovingObject::getHeight() const
	{
		return this->boundingRect_.height;
	}

	std::vector<cv::Point> MovingObject::getContour() const
	{
		return this->contour_;
	}

	cv::Rect MovingObject::getBoudingRect() const
	{
		return this->boundingRect_;
	}
} // namespace cvision
