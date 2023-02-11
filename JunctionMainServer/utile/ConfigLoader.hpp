#ifndef UTILE_CONFIGLOADER_HPP
#define UTILE_CONFIGLOADER_HPP

#include <string>

#include <boost/property_tree/ptree.hpp>                                        
#include <boost/property_tree/json_parser.hpp> 
#include <boost/optional.hpp>

#include "../model/Config.hpp"
#include "utile/GeoCoordinate.hpp"
#include "../common/utile/Logger.hpp"

namespace utile
{
	// TO CHANGE THIS
	using namespace boost::property_tree;
	class ConfigLoader
	{
	private:
		LOGGER("CONFIG-LOADER");

		bool setLaneIPs(const ptree& jsonRoot, model::Config& config);
		bool setUsedLane(const ptree& jsonRoot, model::Config& config);
		bool setMaxWaitingTime(const ptree& jsonRoot, model::Config& config);
		bool loadLatitude(const ptree& jsonRoot, model::Config& config);
		bool loadLongitude(const ptree& jsonRoot, model::Config& config);
		bool loadLocalServer(const ptree& jsonRoot, model::Config& config);
		void loadMissingRoadIfPresent(const ptree& jsonRoot, model::Config& config);
	public:
		model::Config load(const std::string& pathToConfigFile);
	};
} // namespace utile
#endif // #UTILE_CONFIGLOADER_HPP

