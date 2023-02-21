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
			const std::string ipAdress_;
			const uint16_t port_;
			uint32_t load_;
			const BoundingRectPtr coveredArea_;

		public:
			Proxy() = delete;
			Proxy(const std::string& ipAdress,
				const uint16_t& port,
				const uint32_t& load,
				const std::shared_ptr<BoundingRect>& coveredArea);
			~Proxy() noexcept = default;
			std::string getIpAdress() const;
			uint16_t getPort() const;
			uint32_t getLoad() const;
			uint32_t updateLoad(bool connecting);
			BoundingRectPtr getCoveredArea() const;
			bool isContained(const GeoCoordinate<DecimalCoordinate>& point) const;
		};
	} // namespace db
} // namespace common
#endif // #COMMON_DB_PROXY_HPP
