#include "Proxy.hpp"

namespace common
{
	namespace db
	{
		Proxy::Proxy(const uint32_t& id,
			const uint32_t& load,
			const std::shared_ptr<BoundingRect>& coveredArea,
			const std::vector<std::shared_ptr<Junction>>monitoredJunctions) :
			id_(id),
			load_(load),
			coveredArea_(coveredArea),
			monitoredJunctions_(monitoredJunctions)
		{

		}
		
		uint32_t Proxy::getId() const
		{
			return id_;
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

		void Proxy::setMonitoredJunctions(const std::vector< std::shared_ptr<Junction> >& monitoredJunctions)
		{
			monitoredJunctions_ = monitoredJunctions;
		}

		std::vector<JunctionPtr> Proxy::getMonitoredJunctions() const
		{
			return monitoredJunctions_;
		}

	} // namespace db
} // namespace common