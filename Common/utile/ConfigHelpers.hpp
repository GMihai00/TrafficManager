#pragma once
#ifndef COMMON_UTILE_CONFIGHELPERS_HPP
#define COMMON_UTILE_CONFIGHELPERS_HPP

#include <optional>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <stack>
#include <sstream>

#include <nlohmann/json.hpp>

#include "DataTypes.hpp"
#include "GeoCoordinate.hpp"

namespace common
{
	namespace utile
	{
		typedef uint16_t PORT;
		typedef std::string IP_ADRESS;
		typedef std::set<IP_ADRESS> IP_ADRESSES;

		namespace model
		{
			struct JMSConfig
			{
				std::map<LANE, std::string> laneToKeyword;
				uint8_t usingLeftLane;
				uint16_t maxWaitingTime;
				utile::IP_ADRESS serverIp;
				utile::PORT serverPort;
				std::optional<LANE> missingLane = {};
			};

			struct proxy_config_data
			{
				IP_ADRESS ip = "";
				PORT port = 0;
				GeoCoordinate<DecimalCoordinate> boundSW{0., 0.};
				GeoCoordinate<DecimalCoordinate> boundNE{0., 0.};
				std::string dbServer = "";
				std::string dbUsername = "";
				std::string dbPassword = "";

				std::string toString() const
				{
					std::stringstream ss;
					ss << "ip= " << ip << " port= " << port << " server: " << dbServer << 
						" boundSW: " << boundSW.toString() << " boundNE: " << boundNE.toString();

					return ss.str();
				}
			};

		}

		template<typename T>
		bool getData(const nlohmann::json& json, const std::string& key,  T& data)
		{
			auto val = json.find(key);
			if (val == json.end())
			{
				std::cerr << "\"" + key + "\" key missing";
				return false;
			}

			try
			{
				data = val->get<T>();
				return true;
			}
			catch (const std::exception& ec)
			{
				std::cerr << "Invalid data err: " << ec.what();
				return false;
			}
		}

		std::optional<model::JMSConfig> loadJMSConfig(const std::string& pathToConfigFile) noexcept;

		std::stack<std::pair<utile::IP_ADRESS, utile::PORT>> getLastVisitedProxys(const std::string& pathToConfigFile) noexcept;

		bool saveVTConfig(std::stack<std::pair<utile::IP_ADRESS, utile::PORT>> lastVisitedProxys) noexcept;

		std::optional<model::proxy_config_data> loadProxyConfig(const nlohmann::json& data) noexcept;
		std::optional<model::proxy_config_data> loadProxyConfig(const std::string& pathToConfigFile) noexcept;

		std::vector<model::proxy_config_data> loadProxyConfigs(const std::string& pathToConfigFile) noexcept;
	} // namespace utile
} // namespace common
#endif // #COMMON_UTILE_CONFIGHELPERS_HPP