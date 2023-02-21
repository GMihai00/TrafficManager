#include "Proxy.hpp"

namespace common
{
	namespace db
	{
		Proxy::Proxy(const std::string& ipAdress, const uint16_t& port,
			const uint32_t& load,
			const std::shared_ptr<BoundingRect>& coveredArea) :
			ipAdress_(ipAdress),
			port_(port),
			load_(load),
			coveredArea_(coveredArea)
		{

		}

		std::string Proxy::getIpAdress() const
		{
			return ipAdress_;
		}

		uint16_t Proxy::getPort() const
		{
			return port_;
		}

		uint32_t Proxy::getLoad() const
		{
			return load_;
		}

		uint32_t Proxy::updateLoad(bool connecting)
		{
			(connecting == true) ? (load_++) : (load_--);
			return load_;
		}

		BoundingRectPtr Proxy::getCoveredArea() const
		{
			return coveredArea_;
		}

		bool Proxy::isContained(const GeoCoordinate<DecimalCoordinate>& point) const
		{
			return coveredArea_->isContained(point);
		}

	} // namespace db
} // namespace common