#pragma once
#ifndef COMMON_DB_PROXY_HPP
#define COMMON_DB_PROXY_HPP

#include <stdint.h>
#include <memory>
#include <vector>
#include <memory>
#include <optional>

#include "BoundingRect.hpp"
#include "Junction.hpp"

namespace common
{
	namespace db
	{
		class Proxy;
		typedef std::shared_ptr<Proxy> ProxyPtr;

		class Proxy
		{
		protected:
			const uint32_t id_;
			uint32_t load_;
			const BoundingRectPtr coveredArea_;
			std::vector<JunctionPtr> monitoredJunctions_;

		public:
			Proxy() = delete;
			Proxy(const uint32_t& id,
				const uint32_t& load,
				const std::shared_ptr<BoundingRect>& coveredArea,
				const std::vector<std::shared_ptr<Junction>>monitoredJunctions = {});
			~Proxy() noexcept = default;
			uint32_t getId() const;
			uint32_t getLoad() const;
			uint32_t updateLoad(bool connecting);
			BoundingRectPtr getCoveredArea() const;
			bool isContained(const GeoCoordinate<DecimalCoordinate>& point) const;
			void setMonitoredJunctions(const std::vector< std::shared_ptr<Junction> >& monitoredJunctions);
			std::vector<JunctionPtr> 
		};
	} // namespace db
} // namespace common
#endif // #COMMON_DB_PROXY_HPP
