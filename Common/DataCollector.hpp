#pragma once

#include <fstream>
#include <filesystem>
#include <chrono>
#include <map>

#include "utile/Timer.hpp"

namespace common
{
	namespace data
	{
		class DataCollector final
		{
		private:
			void createLogFile();
			void updateLogFile();

		public:
			DataCollector(const std::filesystem::path& loggingDir = ".\\dataCollectorLogs", const uint16_t updateFrequency = 10);
			void registerCars(const uint32_t id, const uint32_t nrCars);
			void unregisterCars(const uint32_t id);
			~DataCollector() noexcept;

		private:
			std::filesystem::path m_loggingDir;
			std::ofstream m_logFile;
			uint32_t m_carCount{ 0 };
			uint16_t m_updateFrequency;
			std::chrono::seconds m_timeRunning{ 0 };
			std::map<uint32_t, std::pair<uint32_t, std::chrono::seconds>> m_carGroupArrivalTimeMap{};
			std::chrono::seconds m_totalTimeCarsWaited{ 0 };
			utile::TimerPtr m_updateTimer;
			utile::IObserverPtr m_observer;
			std::function<void()> m_timerExpireCallback;
		};

	} // namespace data
} // namespace common