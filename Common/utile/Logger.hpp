#pragma once
#ifndef COMMON_UTILS_LOGGER_HPP
#define COMMON_UTILS_LOGGER_HPP

#include <iostream>
#include <string>
#include <cstdlib>
#include <mutex>

#include "Log.hpp"

#define LOGGER(name) common::utile::Logger logger_{name}
#define LOG_SET_NAME(name) logger_.setName(name)
#define LOG_WARN logger_ << common::utile::LogTypes::WRN
#define LOG_ERR  logger_ << common::utile::LogTypes::ERR 
#define LOG_DBG  logger_ << common::utile::LogTypes::DBG
#define LOG_INF  logger_ << common::utile::LogTypes::INF

namespace common
{
    namespace utile
    {
        enum class LogTypes
        {
            WRN,
            ERR,
            DBG,
            INF
        };

        inline std::string toString(const LogTypes& type)
        {
            switch (type)
            {
            case LogTypes::WRN:
                return "WRN";
            case LogTypes::ERR:
                return "ERR";
            case LogTypes::INF:
                return "INF";
            case LogTypes::DBG:
                return "DBG";
            }
            return "";
        }

        class Logger
        {
        protected:
            Log& log_;
            std::string name_;
        public:
            LogTypes type_;
            Logger(const std::string& name,
                const LogTypes type = LogTypes::INF);
            ~Logger() = default;

            template<class T>
            Logger& operator <<(const T& data)
            {
                // std::endl flushing buffer was deleting memory that wasn't printed yet, and when we finally got to printing it, memory was no longer owned by us so invalid memory exception was trown
                log_ << toString(type_) << " [" << name_ << "] " << data <<'\n';
                return *this;
            }
    
            void setName(const std::string& name);

            Logger& operator<<(const LogTypes& type);
        };

    } // namespace utile
} // namespace common
#endif // #COMMON_UTILS_LOGGER_HPP