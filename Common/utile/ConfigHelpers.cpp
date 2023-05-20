#include "ConfigHelpers.hpp"

#include <string>
#include <vector>
#include <stack>
#include <regex>

#include <nlohmann/json.hpp>

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
			bool setLaneKeywords(const nlohmann::json& json, model::JMSConfig& config)
			{
				int count = 0;
				nlohmann::json lanes;
				if (auto it = json.find("lanes"); it == json.end())
				{
					LOG_WARN << "Missing \"lanes\" tag";
					return false;
				}
				else
				{
					lanes = *it;
				}

				std::vector <std::string> directions = { "left", "right", "top", "down" };

				for (auto poz = 0; poz < directions.size(); poz++)
				{
					auto val = lanes.find(directions[poz]);
					if (val == lanes.end())
					{
						LOG_WARN << "Missing lane: " << directions[poz];
						continue;
					}

					try
					{
						auto keyword = val->get<std::string>();
						config.laneToKeyword[common::utile::LANE(poz)] = std::move(keyword);
						count++;
					}
					catch (const std::exception& ec)
					{
						LOG_ERR << "Failed to read data err: " << ec.what();
						return false;
					}
				}
				return count >= 3;
			}

			bool setUsedLane(const nlohmann::json& json, model::JMSConfig& config)
			{
				auto val = json.find("usingLeftLane");
				if (val == json.end())
				{
					LOG_ERR << "\"usingLeftLane\" tag missing";
					return false;
				}

				try
				{
					config.usingLeftLane = val->get<uint8_t>();
					return true;
				}
				catch (const std::exception& ec)
				{
					LOG_ERR << "Invalid data err: " << ec.what();
					return false;
				}
			}
			bool setMaxWaitingTime(const nlohmann::json& json, model::JMSConfig& config)
			{
				auto val = json.find("maxWaitingTime");
				if (val == json.end())
				{
					LOG_ERR << "\"maxWaitingTime\" tag missing";
					return false;
				}

				try
				{
					config.maxWaitingTime = val->get<uint16_t>();
					return true;
				}
				catch (const std::exception& ec)
				{
					LOG_ERR << "Invalid data err: " << ec.what();
					return false;
				}
			}

			void setMissingRoadIfPresent(const nlohmann::json& json, model::JMSConfig& config)
			{
				auto val = json.find("missingLane");
				if (val == json.end())
					return;

				try
				{
					auto lane = val->get<std::string>();

					std::vector <std::string> directions = { "top", "down" , "left", "right" };
					for (int poz = 0; poz < directions.size(); poz++)
					{
						if (lane == directions[poz])
						{
							config.missingLane = (common::utile::LANE)poz;
							return;
						}
					}
					LOG_WARN << "Invalid missingLane value: " << lane;
				}
				catch (const std::exception& ec)
				{
					LOG_ERR << "Invalid data err: " << ec.what();
				}
			}

			bool setServerIp(const nlohmann::json& json, model::JMSConfig& config)
			{
				auto val = json.find("ip");
				if (val == json.end())
				{
					LOG_ERR << "\"ip\" tag missing";
					return false;
				}

				try
				{
					config.serverIp = val->get<std::string>();
					return true;
				}
				catch (const std::exception& ec)
				{
					LOG_ERR << "Invalid data err: " << ec.what();
					return false;
				}
			}

			bool setServerPort(const nlohmann::json& json, model::JMSConfig& config)
			{
				auto val = json.find("port");
				if (val == json.end())
				{
					LOG_ERR << "\"port\" tag missing";
					return false;
				}

				try
				{
					config.serverPort = val->get<uint16_t>();
					return true;
				}
				catch (const std::exception& ec)
				{
					LOG_ERR << "Invalid data err: " << ec.what();
					return false;
				}
			}

			bool setServerEnpoint(const nlohmann::json& json, model::JMSConfig& config)
			{
				auto val = json.find("server");
				if (val == json.end())
				{
					LOG_ERR << "\"server\" tag missing";
					return false;
				}

				return setServerIp(*val, config) && setServerPort(*val, config);
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

			bool setProxyIp(const ptree& jsonRoot, model::proxy_config_data& config)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("ip");
				if (jsonTree == boost::none)
					return false;

				if (const auto& val = jsonTree.get().get_value_optional<std::string>(); val != boost::none)
				{
					config.ip = val.get();
					return true;
				}

				return false;
			}

			bool setProxyPort(const ptree& jsonRoot, model::proxy_config_data& config)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("port");
				if (jsonTree == boost::none)
					return false;

				if (const auto& val = jsonTree.get().get_value_optional<uint16_t>(); val != boost::none)
				{
					config.port = val.get();
					return true;
				}

				return false;
			}

			bool setProxyEnpoint(const ptree& jsonRoot, model::proxy_config_data& config)
			{

				return setProxyIp(jsonRoot, config) && setProxyPort(jsonRoot, config);
			}

			bool setCoordinateLatitude(const ptree& jsonRoot, GeoCoordinate<DecimalCoordinate> coordinate)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("latitude");
				if (jsonTree == boost::none)
					return false;

				if (const auto& val = jsonTree.get().get_value_optional<double>(); val != boost::none)
				{
					coordinate.latitude = val.get();
					return true;
				}

				return false;
			}

			bool setCoordinateLongitude(const ptree& jsonRoot, GeoCoordinate<DecimalCoordinate> coordinate)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("longitude");
				if (jsonTree == boost::none)
					return false;

				if (const auto& val = jsonTree.get().get_value_optional<double>(); val != boost::none)
				{
					coordinate.longitude = val.get();
					return true;
				}

				return false;
			}

			boost::optional<GeoCoordinate<DecimalCoordinate>> getCoordinate(const ptree& jsonRoot, std::string name)
			{
				const auto& jsonTree = jsonRoot.get_child_optional(name);
				if (jsonTree == boost::none)
					return boost::none;

				GeoCoordinate<DecimalCoordinate>  coordinate;
				if (!(setCoordinateLatitude(jsonTree.get(), coordinate) && setCoordinateLongitude(jsonTree.get(), coordinate)))
					return boost::none;

				return coordinate;
			}

			bool setProxyCoordinates(const ptree& jsonRoot, model::proxy_config_data& config)
			{
				auto maybeBoundSW = getCoordinate(jsonRoot, "bound_sw");
				auto maybeBoundNE = getCoordinate(jsonRoot, "bound_ne");
				if (maybeBoundSW == boost::none || maybeBoundNE == boost::none)
					return false;

				config.boundSW = maybeBoundSW.get();
				config.boundNE = maybeBoundNE.get();
				return true;
			}

			bool setDbServer(const ptree& jsonRoot, model::proxy_config_data& config)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("server");
				if (jsonTree == boost::none)
					return false;

				if (const auto& val = jsonTree.get().get_value_optional<std::string>(); val != boost::none)
				{
					config.dbServer = val.get();
					return true;
				}

				return false;
			}

			bool setDbUsername(const ptree& jsonRoot, model::proxy_config_data& config)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("username");
				if (jsonTree == boost::none)
					return false;

				if (const auto& val = jsonTree.get().get_value_optional<std::string>(); val != boost::none)
				{
					config.dbUsername = val.get();
					return true;
				}

				return false;
			}

			bool setDbPassword(const ptree& jsonRoot, model::proxy_config_data& config)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("password");
				if (jsonTree == boost::none)
					return false;

				if (const auto& val = jsonTree.get().get_value_optional<std::string>(); val != boost::none)
				{
					config.dbPassword = val.get();
					return true;
				}

				return false;
			}

			bool setDbCredentials(const ptree& jsonRoot, model::proxy_config_data& config)
			{
				return setDbUsername(jsonRoot, config) && setDbPassword(jsonRoot, config);
			}

			bool setDbConnectionData(const ptree& jsonRoot, model::proxy_config_data& config)
			{
				const auto& jsonTree = jsonRoot.get_child_optional("db");
				if (jsonTree == boost::none)
					return false;

				return setDbServer(jsonTree.get(), config) && setDbCredentials(jsonTree.get(), config);
			}
		}

		std::optional<model::JMSConfig> loadJMSConfig(const std::string& pathToConfigFile) noexcept
		{
			model::JMSConfig config;
			nlohmann::json data;

			std::ifstream input_file(pathToConfigFile);

			if (!input_file.is_open())
			{
				LOG_ERR << "Failed to open: " << pathToConfigFile;
				return {};
			}

			data = nlohmann::json::parse(input_file);

			input_file.close();

			if (!details::setLaneKeywords(data, config))
			{
				LOG_WARN << "TOs keywords not found. The server will read messages only from VTs";
			}

			if (!details::setUsedLane(data, config))
			{
				LOG_WARN << "Lane not specified, defaulting to right lane";
				config.usingLeftLane = 0;
			}

			if (!details::setMaxWaitingTime(data, config))
			{
				LOG_WARN << "Maximum waiting time not specified, defaulting to 300s";
				config.maxWaitingTime = 300;
			}

			if (!details::setServerEnpoint(data, config))
			{
				LOG_ERR << "Ip and Port not specified";
				return {};
			}

			details::setMissingRoadIfPresent(data, config);

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

		std::vector<model::proxy_config_data> loadProxyConfigs(const std::string& pathToConfigFile) noexcept
		{
			std::vector<model::proxy_config_data> rez;
			ptree jsonRoot;

			read_json(pathToConfigFile, jsonRoot);
			const auto& jsonTree = jsonRoot.get_child_optional("proxys");

			if (jsonTree == boost::none)
			{
				return {};
			}

			for (const auto& item : jsonTree.get())
			{
				model::proxy_config_data config;
				if (!details::setProxyEnpoint(item.second, config))
				{
					LOG_WARN << "Invalid config data. Missing proxy endpoint";
					continue;
				}

				if (!details::setProxyCoordinates(item.second, config))
				{
					LOG_WARN << "Invalid config data. Missing proxy coordinates";
					continue;
				}

				if (!details::setDbConnectionData(item.second, config))
				{
					LOG_WARN << "Invalid config data. Missing DB Connection Data";
					continue;
				}

				rez.push_back(std::move(config));
			}

			return rez;
		}
	} // namespace utile
} // namespace common