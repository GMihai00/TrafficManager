#pragma once
#ifndef MODEL_CONFIG_HPP
#define MODEL_CONFIG_HPP

#include <string>
#include <map>
#include <optional>
#include "utile/GeoCoordinate.hpp"
#include "utile/DataTypes.hpp"
#include "utile/IPCDataTypes.hpp"

namespace model
{
	struct Config
	{
		// KEYWORDS HAVE FIXED SIDE
		std::map<common::utile::LANE, std::string> laneToKeyword;
		bool usingLeftLane;
		uint16_t maxWaitingTime;
		ipc::utile::IP_ADRESS serverIp;
		ipc::utile::PORT serverPort;
		std::optional<common::utile::LANE> missingLane;
	};
}
#endif // #MODEL_CONFIG_HPP
