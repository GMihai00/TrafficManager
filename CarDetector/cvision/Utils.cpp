#include "Utils.hpp"
namespace model
{
	namespace cvision
	{
		double distanceBetweenPoints(cv::Point point1, cv::Point point2)
		{
			int intX = abs(point1.x - point2.x);
			int intY = abs(point1.y - point2.y);

			return(sqrt(pow(intX, 2) + pow(intY, 2)));
		}
	} // namespace cvision
} // namespace model