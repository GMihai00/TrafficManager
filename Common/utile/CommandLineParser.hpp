#pragma once
#ifndef COMMON_UTILE_COMMANDLINEPARSER_HPP
#define COMMON_UTILE_COMMANDLINEPARSER_HPP

#include <vector>
#include <string_view>
#include <optional>

namespace common
{
	namespace utile
	{
		class CommandLineParser
		{
		private:
			const std::vector<std::string_view> args_;
		public:
			CommandLineParser() = delete;
			CommandLineParser(int argc, char* argv[]);
			~CommandLineParser() noexcept = default;

			std::optional<std::string_view> getOption(const std::string_view& optionName) const;
			bool isFlagSet(const std::string_view& optionName) const;
		};

	} // namespace utile
} // namespace common
#endif // #COMMON_UTILE_COMMANDLINEPARSER_HPP