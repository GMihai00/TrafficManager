#include "ConfigHandler.hpp"

namespace utile
{

	std::stack<std::pair<ipc::utile::IP_ADRESS, ipc::utile::PORT>>
		ConfigHandler::getLastVisitedProxys(const std::string& pathToConfigFile) noexcept
	{
		std::stack<std::pair<ipc::utile::IP_ADRESS, ipc::utile::PORT>> rez;

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
		if (jsonTree != boost::none)
		{
			LOG_ERR << "Main key missing from config file";
			return {};
		}
		
		for (auto& item : jsonTree.get())
		{
			auto keyIp_adress = jsonRoot.get_child_optional("ip_adress");
			auto keyPort = jsonRoot.get_child_optional("port");
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

	bool ConfigHandler::saveConfig(std::stack<std::pair<ipc::utile::IP_ADRESS, ipc::utile::PORT>> lastVisitedProxys) noexcept
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
		write_jsonEx("VTConfig.json", jsonRoot);
	}
};