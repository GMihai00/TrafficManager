#ifndef MODEL_CONFIG
#define MODEL_CONFIG

#include <string>
#include <map>
#include <boost/optional.hpp>
#include "GeoCoordinate.hpp"

namespace model
{
	struct Config
	{
		// MANDATORY TO AVOID EXPLOITS
		std::map<uint8_t, std::string> laneToIPAdress;
		bool usingLeftLane;
		uint16_t maxWaitingTime;
		model::GeoCoordinate latitude;
		model::GeoCoordinate longitude;
		std::string localProxyServer;
		boost::optional<uint8_t> missingLane;
	};
}
#endif // !MODEL_CONFIG
