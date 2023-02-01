#ifndef COMMON_UTILS_LOGGER_HPP
#define COMMON_UTILS_LOGGER_HPP

#include <iostream>
#include <string>
#include <cstdlib>

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

    inline std::ostream& operator<<(std::ostream & os, LogTypes & type)
    {
        switch (type)
        {
        case LogTypes::WRN:
            os << "WRN";
            break;
        case LogTypes::ERR:
            os << "ERR";
            break;
        case LogTypes::INF:
            os << "INF";
            break;
        case LogTypes::DBG:
            os << "DBG";
            break;
        }
        return os;
    }

    class Logger
    {
    protected:
        std::ostream& os_;
        std::ostream& oserr_;
        std::string name_;
        LogTypes type_;
    public:
        Logger(const std::string& name,
            const LogTypes type = LogTypes::INF,
            std::ostream& os = std::cout,
            std::ostream& oserr = std::cerr) :
            os_(os),
            oserr_(oserr)
        {
            this->name_ = name;
            this->type_ = type;
        }
        ~Logger() = default;

        template<typename T>
        friend std::ostream& operator << (Logger& logger, T data)
        {
             // TODO: SOMEHOW FLAG NOT SET 
             if (logger.type_ == LogTypes::DBG &&
                 !(std::getenv("LOGGER_DEBUG") && std::getenv("LOGGER_DEBUG") == "TRUE"))
             {
                 return logger.os_;
             }

            if (logger.type_ == LogTypes::ERR)
            {
                logger.oserr_ << logger.type_
                    << " [" << logger.name_ << "] "
                    << data;
                
                return logger.oserr_;
            }
            else
            {
                logger.os_ << logger.type_
                    << " [" << logger.name_ << "] "
                    << data;
    
                return logger.os_;
            }
        }
    
        void setName(const std::string& name)
        {
            this->name_ = name;
        }

        friend Logger& operator<<(Logger& logger, LogTypes type);
    };

    inline Logger& operator<<(Logger& logger, LogTypes type)
    {
        logger.type_ = type;
        if (logger.type_ == LogTypes::ERR)
        {
            logger.oserr_ << '\n';
        }
        else
        {
            logger.os_ << '\n';
        }
        return logger;
    }
} // namespace utile
} // namespace common
#endif // #COMMON_UTILS_LOGGER_HPP