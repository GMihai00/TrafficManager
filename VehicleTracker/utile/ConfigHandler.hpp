#pragma once
#ifndef UTILE_CONFIGHANDLER_HPP
#define UTILE_CONFIGHANDLER_HPP

#include <string>
#include <vector>
#include <stack>
#include <regex>

#include <boost/property_tree/ptree.hpp>                                        
#include <boost/property_tree/json_parser.hpp> 
#include <boost/optional.hpp>

#include "utile/Logger.hpp"
#include "utile/IPCDataTypes.hpp"

namespace utile
{
	using namespace boost::property_tree;
	namespace details
	{
		inline bool write_jsonEx(const std::string& path, const ptree& ptree)
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

	std::stack<std::pair<ipc::utile::IP_ADRESS, ipc::utile::PORT>> getLastVisitedProxys(const std::string& pathToConfigFile) noexcept;

	bool saveConfig(std::stack<std::pair<ipc::utile::IP_ADRESS, ipc::utile::PORT>> lastVisitedProxys) noexcept;

} // namespace utile
#endif // #UTILE_CONFIGHANDLER_HPP