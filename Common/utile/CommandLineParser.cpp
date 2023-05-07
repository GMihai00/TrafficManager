#include "CommandLineParser.hpp"

#include <algorithm>
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

		bool CommandLineParser::isFlagSet(const std::string_view& optionName) const
		{
			return std::any_of(args_.begin(), args_.end(), [&optionName](const auto& arg) { return arg == optionName; });
		}

	} // namespace utile
} // namespace common