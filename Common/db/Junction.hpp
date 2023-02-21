#pragma once
#ifndef COMMON_DB_JUNCTION_HPP
#define COMMON_DB_JUNCTION_HPP

#include <memory>

#include "BoundingRect.hpp"

#include <memory>

namespace common
{
	namespace db
	{
		class Junction
		{
		protected:
			const std::string ipAdress_;
			const uint16_t port_;
			const BoundingRectPtr coveredArea_;
		public:
			Junction() = delete;
			Junction(const std::string& ipAdress, const uint16_t& port, const std::shared_ptr<BoundingRect> coveredArea);
			~Junction() noexcept = default;
			bool isContained(const GeoCoordinate<DecimalCoordinate>& point) const;
			bool isPassing(const GeoCoordinate<DecimalCoordinate> pointA, const GeoCoordinate<DecimalCoordinate> pointB) const;
			bool isPassing(const GeoCoordinate<DecimalCoordinate> point, const LANE direction) const;
			bool passedJunction(const GeoCoordinate<DecimalCoordinate>& pointA, const GeoCoordinate<DecimalCoordinate>& pointB) const;
			std::string getIpAdress() const;
			uint16_t getPort() const;
			GeoCoordinate<DecimalCoordinate> getCenter() const;
		};
		typedef std::shared_ptr<Junction> JunctionPtr;
	} // namespace db
} // namespace common
#endif // #COMMON_DB_JUNCTION_HPP
