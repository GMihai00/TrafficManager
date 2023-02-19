#include "Proxy.hpp"

namespace common
{
	namespace db
	{
		Proxy::Proxy(const std::shared_ptr<BoundingRect>& coveredArea,
			const uint32_t& load,
			const std::vector<std::shared_ptr<Junction>>monitoredJunctions,
			const uint32_t& id,
			const std::shared_ptr<Proxy> parent) :
			coveredArea_(coveredArea),
			load_(load),
			monitoredJunctions_(monitoredJunctions),
			id_(id),
			parent_(parent)
		{

		}
		
		bool Proxy::isContained(const GeoCoordinate<DecimalCoordinate>& point) const
		{
			return coveredArea_->isContained(point);
		}

		std::shared_ptr<Proxy> Proxy::getParent() const
		{
			return parent_;
		}

		void Proxy::setMonitoredJunctions(const std::vector< std::shared_ptr<Junction> >& monitoredJunctions)
		{
			this->monitoredJunctions_ = monitoredJunctions;
		}

	} // namespace db
} // namespace common