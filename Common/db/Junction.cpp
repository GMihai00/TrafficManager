#include "Junction.hpp"

namespace common
{
	namespace db
	{
		Junction::Junction(const std::string& ipAdress, const uint16_t& port, const std::shared_ptr<BoundingRect> coveredArea) :
			ipAdress_(ipAdress),
			port_(port),
			coveredArea_(coveredArea)
		{

		}
		
		bool Junction::isContained(const GeoCoordinate<DecimalCoordinate>& point) const
		{
			return coveredArea_->isContained(point);
		}

		bool Junction::isPassing(const GeoCoordinate<DecimalCoordinate> pointA, const GeoCoordinate<DecimalCoordinate> pointB) const
		{
			return (calculateDirection(pointA, pointB) == calculateDirection(pointB, coveredArea_->getCenter()));
		}

		bool Junction::passedJunction(const GeoCoordinate<DecimalCoordinate>& pointA, const GeoCoordinate<DecimalCoordinate>& pointB) const
		{
			return coveredArea_->passedCenter(pointA, pointB);
		}

		std::string Junction::getIpAdress() const
		{
			return ipAdress_;
		}

		uint16_t Junction::getPort() const
		{
			return port_;
		}
	} // namespace db
} // namespace common