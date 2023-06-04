#include "ConfigHelpers.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <stack>

#include "Logger.hpp"

namespace common
{
	namespace utile
	{
		LOGGER("CONFIG");

		namespace details
		{
			bool readJson(const std::string& pathToConfigFile, nlohmann::json& data)
			{
				std::ifstream input_file(pathToConfigFile);

				if (!input_file.is_open())
				{
					LOG_ERR << "Failed to open: " << pathToConfigFile;
					return false;
				}

				try
				{
					data = nlohmann::json::parse(input_file);
				}
				catch (const std::exception& ec)
				{
					LOG_ERR << "Failed to read config file, err = " << ec.what();
				}

				input_file.close();
				return true;
			}

			bool setLaneKeywords(const nlohmann::json& json, model::JMSConfig& config)
			{
				int count = 0;
				nlohmann::json lanes;
				if (auto it = json.find("lanes"); it == json.end())
				{
					LOG_WARN << "Missing \"lanes\" key";
					return false;
				}
				else
				{
					lanes = *it;
				}

				std::vector <std::string> directions = { "right", "left", "top", "down" };

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
				return getData(json, "usingLeftLane", config.usingLeftLane);
			}

			bool setMaxWaitingTime(const nlohmann::json& json, model::JMSConfig& config)
			{
				return getData(json, "maxWaitingTime", config.maxWaitingTime);
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
				return getData(json, "ip", config.serverIp);
			}

			bool setServerPort(const nlohmann::json& json, model::JMSConfig& config)
			{
				return getData(json, "port", config.serverPort);
			}

			bool setServerEnpoint(const nlohmann::json& json, model::JMSConfig& config)
			{
				auto val = json.find("server");
				if (val == json.end())
				{
					LOG_ERR << "\"server\" key missing";
					return false;
				}

				return setServerIp(*val, config) && setServerPort(*val, config);
			}

			bool setProxyIp(const nlohmann::json& json, model::proxy_config_data& config)
			{
				return getData(json, "ip", config.ip);
			}

			bool setProxyPort(const nlohmann::json& json, model::proxy_config_data& config)
			{
				return getData(json, "port", config.port);
			}

			bool setProxyEnpoint(const nlohmann::json& json, model::proxy_config_data& config)
			{
				return setProxyIp(json, config) && setProxyPort(json, config);
			}

			bool setCoordinateLatitude(const nlohmann::json& json, GeoCoordinate<DecimalCoordinate>& coordinate)
			{
				return getData(json, "latitude", coordinate.latitude);
			}

			bool setCoordinateLongitude(const nlohmann::json& json, GeoCoordinate<DecimalCoordinate>& coordinate)
			{
				return getData(json, "longitude", coordinate.longitude);
			}

			std::optional<GeoCoordinate<DecimalCoordinate>> getCoordinate(const nlohmann::json& json, std::string name)
			{
				auto val = json.find(name);
				if (val == json.end())
				{
					LOG_ERR << "\"" + name + "\" key missing";
					return {};
				}

				GeoCoordinate<DecimalCoordinate> coordinate{ 0., 0. };
				if (!(setCoordinateLatitude(*val, coordinate) && setCoordinateLongitude(*val, coordinate)))
					return {};

				return coordinate;
			}

			bool setProxyCoordinates(const nlohmann::json& json, model::proxy_config_data& config)
			{
				auto maybeBoundSW = getCoordinate(json, "bound_sw");
				auto maybeBoundNE = getCoordinate(json, "bound_ne");
				if (!maybeBoundSW.has_value() || !maybeBoundNE.has_value())
					return false;

				config.boundSW = maybeBoundSW.value();
				config.boundNE = maybeBoundNE.value();
				return true;
			}

			bool setDbServer(const nlohmann::json& json, model::proxy_config_data& config)
			{
				return getData(json, "server", config.dbServer);
			}

			bool setDbUsername(const nlohmann::json& json, model::proxy_config_data& config)
			{
				return getData(json, "username", config.dbUsername);
			}

			bool setDbPassword(const nlohmann::json& json, model::proxy_config_data& config)
			{
				return getData(json, "password", config.dbPassword);
			}

			bool setDbCredentials(const nlohmann::json& json, model::proxy_config_data& config)
			{
				return setDbUsername(json, config) && setDbPassword(json, config);
			}

			bool setDbConnectionData(const nlohmann::json& json, model::proxy_config_data& config)
			{
				auto val = json.find("db");
				if (val == json.end())
				{
					LOG_ERR << "\"db\" key missing";
					return false;
				}

				return setDbServer(*val, config) && setDbCredentials(*val, config);
			}
		}

		std::optional<model::JMSConfig> loadJMSConfig(const std::string& pathToConfigFile) noexcept
		{
			model::JMSConfig config;
			nlohmann::json data;

			if (!details::readJson(pathToConfigFile, data))
			{
				return {};
			}

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

			nlohmann::json data;

			if (!details::readJson(pathToConfigFile, data))
			{
				return {};
			}

			auto proxys = data.find("proxys");
			if (proxys == data.end())
			{
				LOG_ERR << "\"proxys\" key missing from config file";
				return {};
			}

			for (const auto& item : *proxys)
			{
				auto ip_adress = item.find("ip_adress");
				auto port = item.find("port");

				if (ip_adress != item.end() && port != item.end())
				{
					try
					{
						rez.push({ ip_adress->get<std::string>(), port->get<uint16_t>() });
					}
					catch(const std::exception& err)
					{
						LOG_WARN << "Invalid data present in config file, err=" << err.what();
					}
				}
				else
				{
					LOG_WARN << "Invalid data present, keys missing";
				}
			}

			return rez;
		}

		bool saveVTConfig(std::stack<std::pair<utile::IP_ADRESS, utile::PORT>> lastVisitedProxys) noexcept
		{
			nlohmann::json proxys;

			while (!lastVisitedProxys.empty())
			{
				auto ipPortPair = lastVisitedProxys.top();
				lastVisitedProxys.pop();
				nlohmann::json elem;
				elem.emplace("ip_adress", ipPortPair.first);
				elem.emplace("port", ipPortPair.second);
				proxys.push_back(elem);
			}
			auto data = nlohmann::json{ "proxys", std::move(proxys)};


			std::ofstream config("VTConfig.json");

			if (!config.is_open())
			{
				LOG_ERR << "Failed to open file VTConfig.json";
				return false;
			}
			try
			{
				config << data.dump(4);
				config.close();

				return true;
			}
			catch (const std::exception& err)
			{
				LOG_ERR << "Failed to backup config data err= " << err.what();
				config.close();
				return false;
			}
		}

		std::optional<model::proxy_config_data> loadProxyConfig(const nlohmann::json& data) noexcept
		{
			model::proxy_config_data config;
			if (!details::setProxyEnpoint(data, config))
			{
				LOG_WARN << "Invalid config data. Missing proxy endpoint";
				return std::nullopt;
			}

			if (!details::setProxyCoordinates(data, config))
			{
				LOG_WARN << "Invalid config data. Missing proxy coordinates";
				return std::nullopt;
			}

			if (!details::setDbConnectionData(data, config))
			{
				LOG_WARN << "Invalid config data. Missing DB Connection Data";
				return std::nullopt;
			}

			return config;
		}

		std::optional<model::proxy_config_data> loadProxyConfig(const std::string& pathToConfigFile) noexcept
		{
			nlohmann::json data;

			if (!details::readJson(pathToConfigFile, data))
			{
				return std::nullopt;
			}

			model::proxy_config_data config;
			if (!details::setProxyEnpoint(data, config))
			{
				LOG_WARN << "Invalid config data. Missing proxy endpoint";
				return std::nullopt;
			}

			if (!details::setProxyCoordinates(data, config))
			{
				LOG_WARN << "Invalid config data. Missing proxy coordinates";
				return std::nullopt;
			}

			if (!details::setDbConnectionData(data, config))
			{
				LOG_WARN << "Invalid config data. Missing DB Connection Data";
				return std::nullopt;
			}

			return config;
		}

		std::vector<model::proxy_config_data> loadProxyConfigs(const std::string& pathToConfigFile) noexcept
		{
			std::vector<model::proxy_config_data> rez;
			nlohmann::json data;

			if (!details::readJson(pathToConfigFile, data))
			{
				return {};
			}

			auto proxys = data.find("proxys");

			if (proxys == data.end())
			{
				return {};
			}

			for (const auto& item : *proxys)
			{
				auto config = loadProxyConfig(item);

				if (config.has_value())
					rez.push_back(config.value());
			}

			return rez;
		}
	} // namespace utile
} // namespace common