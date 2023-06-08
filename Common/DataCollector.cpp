#include "DataCollector.hpp"

#include <exception>
#include <string>
#include <iomanip>
#include <sstream>

#include "utile/Observer.hpp"

namespace common
{
	namespace data
	{
		void DataCollector::createLogFile()
		{
			auto now = std::chrono::system_clock::now();
			std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

			std::stringstream ss;
			tm timeStruct;
			localtime_s(&timeStruct, &currentTime);
			ss << std::put_time(&timeStruct, "%Y%m%d_%H%M%S");

			std::string fileName = ss.str() + ".log";
			
			// OK to overwrite
			m_logFile.open(m_loggingDir / fileName);

			if (!m_logFile.is_open())
				throw std::runtime_error("Failed to create logging file");
		}

		void DataCollector::updateLogFile()
		{
			if (m_carCount != 0)
			{
				// simple format to be able to easily plot it using latex
				m_logFile << (m_totalTimeCarsWaited.count() * 1.0) / m_carCount << " " << m_carCount << std::endl;
			}

			if (m_updateTimer && m_updateTimer->hasExpired())
			{
				m_timeRunning += std::chrono::seconds(m_updateFrequency);
				m_updateTimer->resetTimer(m_updateFrequency);
			}
		}

		DataCollector::DataCollector(const std::filesystem::path& loggingDir, const uint16_t updateFrequency)
			: m_loggingDir(loggingDir)
			, m_updateFrequency(updateFrequency)
		{
			std::error_code ec;
			if (!std::filesystem::is_directory(loggingDir, ec))
			{
				if (!std::filesystem::create_directory(loggingDir, ec))
					throw std::runtime_error(loggingDir.string() + " failed to create dir ec= " + std::to_string(ec.value()));
			}

			createLogFile();

			m_timerExpireCallback = std::bind(&DataCollector::updateLogFile, this);

			m_observer = std::make_shared<utile::Observer>(m_timerExpireCallback);

			m_updateTimer = std::make_shared<utile::Timer>(m_updateFrequency);
			m_updateTimer->subscribe(m_observer);
		}

		void DataCollector::registerCars(const uint32_t id, const uint32_t nrCars)
		{
			if (m_carGroupArrivalTimeMap.find(id) == m_carGroupArrivalTimeMap.end())
			{
				m_carGroupArrivalTimeMap.emplace(id, std::pair{nrCars, m_timeRunning + std::chrono::seconds(m_updateFrequency - m_updateTimer->getTimeLeft()) });
			}
		}

		void DataCollector::unregisterCars(const uint32_t id)
		{
			if (auto it = m_carGroupArrivalTimeMap.find(id); it != m_carGroupArrivalTimeMap.end())
			{
				m_totalTimeCarsWaited += m_timeRunning + std::chrono::seconds(m_updateFrequency - m_updateTimer->getTimeLeft()) - it->second.second;
				m_carCount += it->second.first;
				m_carGroupArrivalTimeMap.erase(it);
			}
		}

		DataCollector::~DataCollector() noexcept
		{
			updateLogFile();

			if (m_logFile.is_open())
			{
				m_logFile.close();
			}
		}

	} // namespace data
} // namespace common