#ifndef MODEL_CONFIG_HPP
#define MODEL_CONFIG_HPP

#include <string>
#include <map>
#include <boost/optional.hpp>
#include "GeoCoordinate.hpp"
#include "utile/DataTypes.hpp"
#include "utile/IPCDataTypes.hpp"

namespace model
{
	struct Config
	{
		// MANDATORY TO AVOID EXPLOITS
		std::map<common::utile::LANE, ipc::utile::IP_ADRESS> laneToIPAdress;
		bool usingLeftLane;
		uint16_t maxWaitingTime;
		model::GeoCoordinate latitude;
		model::GeoCoordinate longitude;
		ipc::utile::IP_ADRESS localProxyServer;
		boost::optional<common::utile::LANE> missingLane;
	};
}
#endif // #MODEL_CONFIG_HPP
