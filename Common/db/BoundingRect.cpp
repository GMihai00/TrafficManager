#include "BoundingRect.hpp"

namespace common
{
	namespace db
	{

		BoundingRect::BoundingRect(const GeoCoordinate<DecimalCoordinate>& boundSW, const GeoCoordinate<DecimalCoordinate>& boundNE, const uint32_t& id) :
			boundSW_(boundSW),
			boundNE_(boundNE),
			id_(id)
		{

		}

		bool BoundingRect::isContained(const GeoCoordinate<DecimalCoordinate>& point) const
		{
			return point.latitude >= boundSW_.latitude && point.latitude <= boundNE_.latitude &&
				point.longitude >= boundSW_.longitude && point.longitude <= boundNE_.longitude;
		}

		GeoCoordinate<DecimalCoordinate> BoundingRect::getCenter() const
		{
			GeoCoordinate<DecimalCoordinate> center{};
			center.latitude = (boundSW_.latitude + boundNE_.latitude) / 2.0;
			center.longitude = (boundSW_.longitude + boundNE_.longitude) / 2.0;

			return center;
		}

		bool BoundingRect::passedCenter(const GeoCoordinate<DecimalCoordinate>& pointA, const GeoCoordinate<DecimalCoordinate>& pointB) const
		{
			auto center = getCenter();
			return (pointA.latitude <= center.latitude && center.latitude <= pointB.latitude) ||
				(pointA.latitude >= center.latitude && center.latitude >= pointB.latitude) || 
				(pointA.longitude <= center.longitude && center.longitude <= pointB.latitude) || 
				(pointA.longitude >= center.longitude && center.longitude >= pointB.latitude);
		}

	} // namespace db
} // namespace common