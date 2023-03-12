#pragma once
#ifndef COMMON_UTILE_CONFIGHELPERS_HPP
#define COMMON_UTILE_CONFIGHELPERS_HPP

#include <optional>
#include <string>
#include <set>
#include <map>
#include <stack>

#include "DataTypes.hpp";

namespace common
{
	namespace utile
	{
		// dublicated I know

		typedef uint16_t PORT;
		typedef std::string IP_ADRESS;
		typedef std::set<IP_ADRESS> IP_ADRESSES;

		namespace model
		{
			struct JMSConfig
			{
				std::map<LANE, std::string> laneToKeyword;
				bool usingLeftLane;
				uint16_t maxWaitingTime;
				utile::IP_ADRESS serverIp;
				utile::PORT serverPort;
				std::optional<LANE> missingLane;
			};
		}

		std::optional<model::JMSConfig> loadJMSConfig(const std::string& pathToConfigFile) noexcept;

		std::stack<std::pair<utile::IP_ADRESS, utile::PORT>> getLastVisitedProxys(const std::string& pathToConfigFile) noexcept;

		bool saveVTConfig(std::stack<std::pair<utile::IP_ADRESS, utile::PORT>> lastVisitedProxys) noexcept;

	} // namespace utile
} // namespace common
#endif // #COMMON_UTILE_CONFIGHELPERS_HPP