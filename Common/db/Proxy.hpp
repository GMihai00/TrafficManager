#pragma once
#ifndef COMMON_DB_PROXY_HPP
#define COMMON_DB_PROXY_HPP

#include <stdint.h>
#include <memory>
#include <vector>

#include "BoundingRect.hpp"
#include "Junction.hpp"

namespace common
{
	namespace db
	{
		class Proxy
		{
		protected:
			const uint32_t id_;
			const std::shared_ptr<Proxy> parent_;
			uint32_t load_;
			const std::shared_ptr<BoundingRect> coveredArea_;
			std::vector<std::shared_ptr<Junction>> monitoredJunctions_;

		public:
			Proxy() = delete;
			Proxy(const std::shared_ptr<BoundingRect>& coveredArea,
				const uint32_t& load = 0,
				const std::vector<std::shared_ptr<Junction>>monitoredJunctions = {},
				const uint32_t& id = 0,
				const std::shared_ptr<Proxy> parent = nullptr);
			~Proxy() noexcept = default;
			bool isContained(const GeoCoordinate<DecimalCoordinate>& point) const;
			std::shared_ptr<Proxy> getParent() const;
			void setMonitoredJunctions(const std::vector< std::shared_ptr<Junction> >& monitoredJunctions);
		};
	} // namespace db
} // namespace common
#endif // #COMMON_DB_PROXY_HPP
