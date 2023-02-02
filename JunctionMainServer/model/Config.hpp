#ifndef MODEL_CONFIG_HPP
#define MODEL_CONFIG_HPP

#include <string>
#include <map>
#include <boost/optional.hpp>
#include "GeoCoordinate.hpp"
#include "../utile/DataTypes.hpp";

namespace model
{
	struct Config
	{
		// MANDATORY TO AVOID EXPLOITS
		std::map<utile::LANE, std::string> laneToIPAdress;
		bool usingLeftLane;
		uint16_t maxWaitingTime;
		model::GeoCoordinate latitude;
		model::GeoCoordinate longitude;
		std::string localProxyServer;
		boost::optional<utile::LANE> missingLane;
	};
}
#endif // #MODEL_CONFIG_HPP
