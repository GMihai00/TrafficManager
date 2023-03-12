#include "ConfigHelpers.hpp"

#include <string>
#include <vector>
#include <stack>
#include <regex>

#include <boost/property_tree/ptree.hpp>                                        
#include <boost/property_tree/json_parser.hpp> 
#include <boost/optional.hpp>

#include "Logger.hpp"

#include <boost/algorithm/string/trim.hpp>
#include <cstring>
#include <exception>


namespace common
{
	namespace utile
	{
		using namespace boost::property_tree;

		LOGGER("CONFIG");

		namespace details
		{
			bool setLaneKeywords(const ptree& jsonRoot, model::JMSConfig& config)
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
						config.laneToKeyword[common::utile::LANE(poz)] = jsonTreeDir.get().get_value<std::string>();
						count++;
					}
				}
				return count != 0;
			}

			bool setUsedLane(const ptree& jsonRoot, model::JMSConfig& config)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("usingRightLane");
				if (jsonTree != boost::none && jsonTree.get().get_value_optional<bool>() != boost::none)
				{
					config.usingLeftLane = jsonTree.get().get_value<bool>();
					return true;
				}

				return false;
			}

			bool setMaxWaitingTime(const ptree& jsonRoot, model::JMSConfig& config)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("maxWaitingTime");
				if (jsonTree != boost::none && jsonTree.get().get_value_optional<uint16_t>() != boost::none)
				{
					config.maxWaitingTime = jsonTree.get().get_value<uint16_t>();
					return true;
				}

				return false;
			}

			void setMissingRoadIfPresent(const ptree& jsonRoot, model::JMSConfig& config)
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

			bool setServerIp(const ptree& jsonRoot, model::JMSConfig& config)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("ip");
				if (jsonTree != boost::none && jsonTree.get().get_value_optional<std::string>() != boost::none)
				{
					config.serverIp = jsonTree.get().get_value<std::string>();
					return true;
				}

				return false;
			}

			bool setServerPort(const ptree& jsonRoot, model::JMSConfig& config)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("port");
				if (jsonTree != boost::none && jsonTree.get().get_value_optional<uint16_t>() != boost::none)
				{
					config.serverPort = jsonTree.get().get_value<uint16_t>();
					return true;
				}

				return false;
			}

			bool setServerEnpoint(const ptree& jsonRoot, model::JMSConfig& config)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("server");
				if (jsonTree == boost::none)
				{
					LOG_ERR << "\"server\" tag missing";
					return false;
				}

				return setServerIp(jsonTree.get(), config) && setServerPort(jsonTree.get(), config);
			}

			bool write_jsonEx(const std::string& path, const ptree& ptree)
			{
				std::ostringstream oss;
				write_json(oss, ptree);
				std::regex reg("\\\"([0-9]+\\.{0,1}[0-9]*)\\\"");
				std::string result = std::regex_replace(oss.str(), reg, "$1");

				std::ofstream file;
				try
				{
					file.open(path, std::ofstream::trunc);
					file << result;
					file.close();
					return true;
				}
				catch (...)
				{
					std::cerr << "Failed to write to file";
					return false;
				}
			}
		}

		std::optional<model::JMSConfig> loadJMSConfig(const std::string& pathToConfigFile) noexcept
		{
			model::JMSConfig config;
			ptree jsonRoot;

			read_json(pathToConfigFile, jsonRoot);

			if (!details::setLaneKeywords(jsonRoot, config))
			{
				LOG_WARN << "TOs keywords not found. The server will read messages only from VTs";
			}

			if (!details::setUsedLane(jsonRoot, config))
			{
				LOG_WARN << "Lane not specified, defaulting to right lane";
				config.usingLeftLane = false;
			}

			if (!details::setMaxWaitingTime(jsonRoot, config))
			{
				LOG_WARN << "Maximum waiting time not specified, defaulting to 300s";
				config.maxWaitingTime = 300;
			}

			if (!details::setServerEnpoint(jsonRoot, config))
			{
				LOG_ERR << "Ip and Port not specified";
				return {};
			}

			details::setMissingRoadIfPresent(jsonRoot, config);

			return config;
		}

		std::stack<std::pair<utile::IP_ADRESS, utile::PORT>> getLastVisitedProxys(const std::string& pathToConfigFile) noexcept
		{
			std::stack<std::pair<utile::IP_ADRESS, utile::PORT>> rez;

			ptree jsonRoot;

			try
			{
				read_json(pathToConfigFile, jsonRoot);
			}
			catch (...)
			{
				LOG_ERR << "Failed to read file";
				return {};
			}

			const auto& jsonTree = jsonRoot.get_child_optional("proxys");
			if (jsonTree == boost::none)
			{
				LOG_ERR << "Main key missing from config file";
				return {};
			}

			for (const auto& item : jsonTree.get())
			{
				auto keyIp_adress = item.second.get_child_optional("ip_adress");
				auto keyPort = item.second.get_child_optional("port");
				if (keyIp_adress != boost::none && keyPort != boost::none)
				{
					auto ip_adress = keyIp_adress.get().get_value_optional<std::string>();
					auto port = keyPort.get().get_value_optional<int>();
					if (ip_adress != boost::none && port != boost::none)
					{
						rez.push({ ip_adress.get(), port.get() });
					}
					else
					{
						LOG_WARN << "Invalid data present in config file";
					}
				}
			}

			return rez;
		}

		bool saveVTConfig(std::stack<std::pair<utile::IP_ADRESS, utile::PORT>> lastVisitedProxys) noexcept
		{
			ptree jsonRoot, array;

			while (!lastVisitedProxys.empty())
			{
				auto ipPortPair = lastVisitedProxys.top();
				lastVisitedProxys.pop();
				ptree elem;
				elem.put<std::string>("ip_adress", ipPortPair.first);
				elem.put<int>("port", ipPortPair.second);
				array.push_back(std::make_pair("", elem));
			}

			jsonRoot.put_child("proxys", array);
			return details::write_jsonEx("VTConfig.json", jsonRoot);
		}

	} // namespace utile
} // namespace common