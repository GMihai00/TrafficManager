#pragma once
#ifndef COMMON_DB_BOUNDINGRECT_HPP
#define COMMON_DB_BOUNDINGRECT_HPP

#include <stdint.h>
#include <memory>

#include "../utile/GeoCoordinate.hpp"

namespace common
{
	namespace db
	{
		using namespace utile;
		class BoundingRect
		{
		protected:
			const uint32_t id_;
			const GeoCoordinate<DecimalCoordinate> boundSW_;
			const GeoCoordinate<DecimalCoordinate> boundNE_;
		public:
			BoundingRect() = delete;
			BoundingRect(const GeoCoordinate<DecimalCoordinate>& boundSW, const GeoCoordinate<DecimalCoordinate>& boundNE);
			BoundingRect(const uint32_t& id, const GeoCoordinate<DecimalCoordinate>& boundSW, const GeoCoordinate<DecimalCoordinate>& boundNE);
			~BoundingRect() noexcept = default;
			std::pair<GeoCoordinate<DecimalCoordinate>, GeoCoordinate<DecimalCoordinate>> getBounds() const;
			bool isContained(const GeoCoordinate<DecimalCoordinate>& point) const;
			GeoCoordinate<DecimalCoordinate> getCenter() const;
			bool passedCenter(const GeoCoordinate<DecimalCoordinate>& pointA, const GeoCoordinate<DecimalCoordinate>& pointB) const;
		};
		typedef std::shared_ptr<BoundingRect> BoundingRectPtr;
	} // namespace db
} // namespace common
#endif // #COMMON_DB_BOUNDINGRECT_HPP
