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
			const auto& latitude = stringToPositionalUnit(jsonTree.get().get_value<std::string>());
			if (latitude != boost::none)
			{
				config.coordinates.latitude = latitude.get();
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
			const auto& longitude = stringToPositionalUnit(jsonTree.get().get_value<std::string>());
			if (longitude != boost::none)
			{
				config.coordinates.longitude = longitude.get();
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

	boost::optional<double> ConfigLoader::stringToDouble(std::string& real)
	{
		boost::algorithm::trim(real);
		try
		{
			int integerPart = std::stoi(real);
			const auto next = real.find(".");
			if (next == std::string::npos)
			{
				return integerPart * 1.0;
			}
			const auto& nextValue = real.substr(next, real.size() - next);
			double floatingPart = std::stoi(nextValue);
			while (floatingPart >= 1.0)
				floatingPart /= 10;
			return integerPart * 1.0 + floatingPart;
		}
		catch (const std::exception& e)
		{
			LOG_ERR << e.what();
			return boost::none;
		}
	}

	boost::optional<common::utile::PositionalUnit> ConfigLoader::stringToPositionalUnit(const std::string& positionalUnit)
	{
		common::utile::PositionalUnit rez;
		//EX: "45° 02' 60.0\" N"
		std::vector<std::string> symbloToFind = { "°", "'", "\"" };
		std::vector<double> numbersFound;
		// FIND numeric values
		std::size_t last = 0;
		std::size_t next;
		for (const auto& symbol : symbloToFind)
		{
			next = positionalUnit.find("°");
			if (next == std::string::npos)
			{
				return boost::none;
			}

			auto value = positionalUnit.substr(last, next - last);
			const auto maybeValue = stringToDouble(value);
			if (maybeValue == boost::none)
			{
				return boost::none;
			}

			numbersFound.push_back(maybeValue.get());
		}
		rez.degrees = numbersFound[0];
		rez.minutues = numbersFound[1];
		rez.seconds = numbersFound[2];
		// read last character for direction
		rez.direction = toupper(positionalUnit[positionalUnit.size() - 1]);
		if (!(rez.direction == 'N' || rez.direction == 'S' || rez.direction == 'E' || rez.direction == 'W'))
		{
			return boost::none;
		}
		return rez;
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