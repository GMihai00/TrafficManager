#include "CommandLineParser.hpp"

namespace common
{
	namespace utile
	{

		CommandLineParser::CommandLineParser(int argc, char* argv[]) :
			args_(argv + 1, argv + argc)
		{
		}

		std::optional<std::string_view> CommandLineParser::getOption(const std::string_view& optionName) const
		{
			std::optional<std::string_view> option = {};
			bool found = false;
			for (const auto& arg : args_)
			{
				if (found)
				{
					option = arg;
					break;
				}

				if (arg == optionName)
				{
					found = true;
					continue;
				}
			}

			return option;
		}


	} // namespace utile
} // namespace common