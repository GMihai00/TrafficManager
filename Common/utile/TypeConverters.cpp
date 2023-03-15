#include "TypeConverters.hpp"

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
            // delete wstr; ???
            return rez;
        }
    } // namespace uitle
} // namespace common