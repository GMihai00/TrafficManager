#include "Logger.hpp"

namespace common
{
    namespace utile
    {
        Log g_log;
        Logger::Logger(const std::string& name, const LogTypes type) :
            log_(g_log)
        {
            this->name_ = name;
            this->type_ = type;
        }

        void Logger::setName(const std::string& name)
        {
            this->name_ = name;
        }

        Logger& Logger::operator<<(const LogTypes& type)
        {
            type_ = type;
            return *this;
        }
    } // namespace utile
} // namespace common
