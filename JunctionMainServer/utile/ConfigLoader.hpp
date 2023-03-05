#ifndef UTILE_CONFIGLOADER_HPP
#define UTILE_CONFIGLOADER_HPP

#include <string>
#include <exception>
#include <optional>

#include <boost/property_tree/ptree.hpp>                                        
#include <boost/property_tree/json_parser.hpp> 

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
		bool setServerEnpoint(const ptree& jsonRoot, model::Config& config);
		bool setServerIp(const ptree& jsonRoot, model::Config& config);
		bool setServerPort(const ptree& jsonRoot, model::Config& config);
		void setMissingRoadIfPresent(const ptree& jsonRoot, model::Config& config);
	public:
		std::optional<model::Config> load(const std::string& pathToConfigFile);
		ConfigLoader() = default;
		~ConfigLoader() noexcept = default;
	};
} // namespace utile
#endif // #UTILE_CONFIGLOADER_HPP

