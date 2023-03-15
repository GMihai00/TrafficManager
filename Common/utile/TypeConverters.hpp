#pragma once
#ifndef COMMON_UTILE_TYPECONVERTERS_HPP
#define COMMON_UTILE_TYPECONVERTERS_HPP

#include <string>

namespace common
{
    namespace utile
    {
        std::wstring utf8_to_utf16(const std::string& s) noexcept;
        std::string utf16_to_utf8(const std::wstring& s) noexcept;
        std::wstring utf8_to_utf16(const std::wstring& s) noexcept;
        std::string utf16_to_utf8(const std::string& s) noexcept;
    } // namespace uitle
} // namespace common
#endif // #COMMON_UTILE_TYPECONVERTERS_HPP