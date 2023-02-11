#include "ConfigLoader.hpp"
#include <boost/algorithm/string/trim.hpp>
#include <cstring>
#include <exception>

namespace utile
{
	bool ConfigLoader::setLaneIPs(const ptree& jsonRoot, model::Config& config)
	{
		int count = 0;
		const auto& jsonTree = jsonRoot.get_child_optional("lanes");
		if (jsonTree == boost::none)
		{
			return false;
		}
		
		std::vector <std::string> directions = { "top", "down" , "left", "right" };
		for (auto poz = 0; poz < directions.size(); poz++)
		{
			const auto& jsonTreeDir = jsonTree.get().get_child_optional(directions[poz]);
			if (jsonTreeDir != boost::none && jsonTreeDir.get().get_value_optional<std::string>() != boost::none)
			{
				config.laneToIPAdress[common::utile::LANE(poz)] = jsonTreeDir.get().get_value<std::string>();
				count++;
			}
		}
		return count != 0;
	}

	bool ConfigLoader::setUsedLane(const ptree& jsonRoot, model::Config& config)
	{
		const auto& jsonTree = jsonRoot.get_child_optional("usingRightLane");
		if (jsonTree != boost::none && jsonTree.get().get_value_optional<bool>() != boost::none)
		{
			config.usingLeftLane = jsonTree.get().get_value<bool>();
			return true;
		}

		return false;
	}

	bool ConfigLoader::setMaxWaitingTime(const ptree& jsonRoot, model::Config& config)
	{
		const auto& jsonTree = jsonRoot.get_child_optional("maxWaitingTime");
		if (jsonTree != boost::none && jsonTree.get().get_value_optional<uint16_t>() != boost::none)
		{
			config.maxWaitingTime = jsonTree.get().get_value<uint16_t>();
			return true;
		}

		return false;
	}

	bool ConfigLoader::loadLatitude(const ptree& jsonRoot, model::Config& config)
	{
		const auto& jsonTree = jsonRoot.get_child_optional("latitude");
		if (jsonTree != boost::none && jsonTree.get().get_value_optional<std::string>() != boost::none)
		{
			auto latitude = common::utile::StringToDecimalCoordinates(jsonTree.get().get_value<std::string>());
			if (latitude.has_value())
			{
				config.coordinates.latitude = latitude.value();
				return true;
			}
		}

		return false;
	}

	bool ConfigLoader::loadLongitude(const ptree& jsonRoot, model::Config& config)
	{
		const auto& jsonTree = jsonRoot.get_child_optional("longitude");
		if (jsonTree != boost::none && jsonTree.get().get_value_optional<std::string>() != boost::none)
		{
			auto longitude = common::utile::StringToDecimalCoordinates(jsonTree.get().get_value<std::string>());
			if (longitude.has_value())
			{
				config.coordinates.longitude = longitude.value();
				return true;
			}
		}

		return false;
	}

	bool ConfigLoader::loadLocalServer(const ptree& jsonRoot, model::Config& config)
	{
		const auto& jsonTree = jsonRoot.get_child_optional("localServer");
		if (jsonTree != boost::none && jsonTree.get().get_value_optional<std::string>() != boost::none)
		{
			config.localProxyServer = jsonTree.get().get_value<std::string>();
			return true;
		}

		return false;
	}

	void ConfigLoader::loadMissingRoadIfPresent(const ptree& jsonRoot, model::Config& config)
	{
		std::vector <std::string> directions = { "top", "down" , "left", "right" };
		const auto& jsonTree = jsonRoot.get_child_optional("missingLane");
		if (jsonTree != boost::none && jsonTree.get().get_value_optional<std::string>() != boost::none)
		{
			const auto& direction = jsonTree.get().get_value<std::string>();
			for (int poz = 0; poz < directions.size(); poz++)
			{
				if (direction == directions[poz])
				{
					config.missingLane = (common::utile::LANE)poz;
					return;
				}
			}
			LOG_WARN << "Invalid missingLane value: " << direction;
		}
	}

	model::Config ConfigLoader::load(const std::string& pathToConfigFile)
	{
		model::Config config;
        ptree jsonRoot;

        read_json(pathToConfigFile, jsonRoot);
       
		if (!setLaneIPs(jsonRoot, config))
		{
			LOG_WARN << "TOs not found. The server will read messages only from VTs";
		}

		if (!setUsedLane(jsonRoot, config))
		{
            LOG_WARN << "Lane not specified, defaulting to right lane";
			config.usingLeftLane = false;
        }

		if (!setMaxWaitingTime(jsonRoot, config))
		{
			LOG_WARN << "Maximum waiting time not specified, defaulting to 300s";
			config.maxWaitingTime = 300;
		}


		if (!(loadLatitude(jsonRoot, config) && loadLongitude(jsonRoot, config)))
		{
			LOG_ERR << "Coordinates could not be loaded! " << 
				"Please verify the integrity of your config file";
			exit(1);
		}

		if (!(loadLocalServer(jsonRoot, config)))
		{
			LOG_ERR << "Could not load IP Address of the local server. " <<
				"Please verify the integrity of your config file";
			exit(1);
		}

		loadMissingRoadIfPresent(jsonRoot, config);

		return config;
	}

} // namespace utile