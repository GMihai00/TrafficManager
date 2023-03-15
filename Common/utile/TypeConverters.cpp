#include "TypeConverters.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

namespace common
{
    namespace utile
    {
        std::wstring utf8_to_utf16(const std::string& s) noexcept
        {
            wchar_t* wstr = new wchar_t(s.size());
            size_t outSize;
            mbstowcs_s(&outSize, wstr, s.size() + 1, s.c_str(), s.size());
            auto rez = std::wstring(wstr);
            //delete wstr;
            return rez;
        }

        std::string utf16_to_utf8(const std::wstring& s) noexcept
        {
            char* str = new char(s.size());
            size_t outSize;
            wcstombs_s(&outSize, str, s.size(), s.c_str(), (size_t)(s.size() - 1));
            auto rez = std::string(str);
            //delete str;
            return rez;
        }

        std::wstring utf8_to_utf16(const std::wstring& s) noexcept
        {
            return s;
        }

        std::string utf16_to_utf8(const std::string& s) noexcept
        {
            return s;
        }
    } // namespace uitle
} // namespace common