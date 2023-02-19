#pragma once
#ifndef COMMON_DB_BOUNDINGRECT_HPP
#define COMMON_DB_BOUNDINGRECT_HPP

#include <stdint.h>
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
			BoundingRect(const GeoCoordinate<DecimalCoordinate>& boundSW, const GeoCoordinate<DecimalCoordinate>& boundNE,const uint32_t& id = 0);
			~BoundingRect() noexcept = default;
			bool isContained(const GeoCoordinate<DecimalCoordinate>& point) const;
			GeoCoordinate<DecimalCoordinate> getCenter() const;
			bool passedCenter(const GeoCoordinate<DecimalCoordinate>& pointA, const GeoCoordinate<DecimalCoordinate>& pointB) const;
		};
	} // namespace db
} // namespace common
#endif // #COMMON_DB_BOUNDINGRECT_HPP
