#include <filesystem>
#include <string>
#include <vector>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <array>
#include <string_view>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <clocale>
#include <cstdlib>

#include "utile/Logger.hpp"
#include "utile/GeoCoordinate.hpp"
#include "utile/ConfigHelpers.hpp"
#include "utile/CommandLineParser.hpp"
#include "utile/SignalHandler.hpp"

using namespace common::utile;

LOGGER("TEST-MAIN");

// TO BE ADDED INSIDE COMMON
std::wstring utf8_to_utf16(const std::string& s)
{
    wchar_t* wstr = new wchar_t(s.size());
    size_t outSize;
    mbstowcs_s(&outSize, wstr, s.size() + 1, s.c_str(), s.size());
    auto rez = std::wstring(wstr);
    delete wstr;
    return rez;
}

std::condition_variable g_condVarEnd;
std::vector<PROCESS_INFORMATION> g_runningProcesses;

// TO BE ADDED INSIDE COMMON
std::filesystem::path getWorkingDirectory()
{
    wchar_t pBuf[MAX_PATH]; // limited by Windows
    size_t len = sizeof(pBuf);

    int bytes = GetModuleFileName(NULL, pBuf, len);
    if (!bytes)
    {
        LOG_ERR << "Failed to get path to current running process";
        exit(5);
    }

    return std::filesystem::path(pBuf).parent_path();
}

struct command_line
{
    std::wstring m_exe;
    std::vector<std::wstring> m_arguments;
};

// TO BE ADDED INSIDE COMMON
bool createProcessFromSameDirectory(const command_line& cmd)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    auto workingDir = getWorkingDirectory();
    workingDir /= cmd.m_exe;

    auto command = workingDir.wstring();
    for (const auto& arg : cmd.m_arguments)
    {
        command += L" " + arg;
    }

    // Start the child process. 
    if (!CreateProcess(NULL,   // No module name (use command line)
        LPWSTR(command.c_str()),   // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        LOG_ERR << "CreateProcess failed error: " << GetLastError();
        return false;
    }

    g_runningProcesses.push_back(pi);
    return true;
}

void closeAllProcesses()
{
    for (const auto& pi : g_runningProcesses)
    {
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

// taken from jms file
bool tryToRun4TOsForEachJMS(const model::JMSConfig& config)
{
    std::vector<command_line> commandsToBeRan;

    for (auto lane_keyword_pair : config.laneToKeyword)
    {
        command_line cmd;
        cmd.m_exe = L"TrafficObserver.exe";
        cmd.m_arguments.push_back(L"-h");
        cmd.m_arguments.push_back(utf8_to_utf16(config.serverIp));
        cmd.m_arguments.push_back(L"-p");
        cmd.m_arguments.push_back(std::to_wstring(config.serverPort));
        cmd.m_arguments.push_back(L"-k");
        cmd.m_arguments.push_back(utf8_to_utf16(lane_keyword_pair.second));
        commandsToBeRan.push_back(cmd);
    }

    return std::all_of(commandsToBeRan.begin(), commandsToBeRan.end(), [](const auto& comand) { return createProcessFromSameDirectory(comand); });
}

// TO DO: Read from a json file
bool loadProxysFromConfigFile(const std::filesystem::path& configPath)
{
    return false;
}

bool runProxy(const IP_ADRESS& serverIp, 
	const PORT& port, 
	GeoCoordinate<DecimalCoordinate> boundSW,
	GeoCoordinate<DecimalCoordinate> boundNe,
	std::string dbServer,
	std::string dbusername,
	std::string dbpassword)
{
    command_line comand;

    createProcessFromSameDirectory(comand);
}

bool runJMSForAllConfigs(const std::filesystem::path& jmsConfigDir)
{
    std::vector<command_line> commandsToBeRan;

    std::error_code ec;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(jmsConfigDir, ec))
    {
        if (std::filesystem::is_regular_file(entry, ec))
        {
            command_line cmd;
            cmd.m_exe = L"JunctionMainServer.exe";
            cmd.m_arguments.push_back(L"-conf");
            cmd.m_arguments.push_back(entry.path().wstring());
            commandsToBeRan.push_back(cmd);

            auto config = loadJMSConfig(entry.path().string()); // IN CASE FAULTY FILES ARE PRESENT
            if (config.has_value())
            {
                commandsToBeRan.push_back(cmd);
                if (!tryToRun4TOsForEachJMS(config.value()))
                {
                    LOG_WARN << "Failed to create TOs for junction";
                }
            }
        }
    }

    return std::all_of(commandsToBeRan.begin(), commandsToBeRan.end(), [](const auto& comand) { return createProcessFromSameDirectory(comand); });
}

// Data generate with the help of https://www.nmeagen.org/
bool runVTForEachGPS(const std::filesystem::path& gpsDataDir, const std::filesystem::path& configPath)
{
    std::vector<command_line> commandsToBeRan;

    std::error_code ec;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(gpsDataDir, ec))
    {
        if (std::filesystem::is_regular_file(entry, ec))
        {
            command_line cmd;
            cmd.m_exe = L"VehicleTracker.exe";
            cmd.m_arguments.push_back(L"-gps_f");
            cmd.m_arguments.push_back(entry.path().wstring());
            cmd.m_arguments.push_back(L"-conf");
            cmd.m_arguments.push_back(configPath.wstring());
            commandsToBeRan.push_back(cmd);
        }
    }

    return std::all_of(commandsToBeRan.begin(), commandsToBeRan.end(), [](const auto& comand) { return createProcessFromSameDirectory(comand); });
}

std::optional<std::filesystem::path> getGPSDataDir(const CommandLineParser& commandLine)
{
    constexpr std::array<std::string_view, 2> optionNames = { "-gps_d", "--gps_dir" };

    for (const auto& optionName : optionNames)
    {
        auto option = commandLine.getOption(optionName);
        if (option.has_value())
        {
            std::error_code ec;
            auto dir = std::filesystem::path(std::string(option.value()));
            if (std::filesystem::is_directory(dir, ec))
            {
                return dir;
            }
            return {};
        }
    }

    return {};
}

std::optional<std::filesystem::path> getVtConfigFile(const CommandLineParser& commandLine)
{
    constexpr std::array<std::string_view, 2> optionNames = { "-vt_conf", "--vehicletracker_config" };

    for (const auto& optionName : optionNames)
    {
        auto option = commandLine.getOption(optionName);
        if (option.has_value())
        {
            std::error_code ec;
            auto file = std::filesystem::path(std::string(option.value()));
            if (std::filesystem::is_regular_file(file, ec))
            {
                return file;
            }
            return {};
        }
    }

    return {};
}

std::optional<std::filesystem::path> getJMSConfigsDir(const CommandLineParser& commandLine)
{
    constexpr std::array<std::string_view, 2> optionNames = { "-jms_d", "--junctionmainserver_directory" };

    for (const auto& optionName : optionNames)
    {
        auto option = commandLine.getOption(optionName);
        if (option.has_value())
        {
            std::error_code ec;
            auto dir = std::filesystem::path(std::string(option.value()));
            if (std::filesystem::is_directory(dir, ec))
            {
                return dir;
            }
            return {};
        }
    }

    return {};
}


std::optional<std::filesystem::path> getProxyConfigFile(const CommandLineParser& commandLine)
{
    constexpr std::array<std::string_view, 2> optionNames = { "-p_conf", "--proxy_config" };

    for (const auto& optionName : optionNames)
    {
        auto option = commandLine.getOption(optionName);
        if (option.has_value())
        {
            std::error_code ec;
            auto file = std::filesystem::path(std::string(option.value()));
            if (std::filesystem::is_regular_file(file, ec))
            {
                return file;
            }
            return {};
        }
    }

    return {};
}

int main(int argc, char* argv[])
{

    SignalHandler sigHandler{};
    sigHandler.setAction(SIGINT, [](int singal)
        {
            g_condVarEnd.notify_one();
        });
    sigHandler.setAction(SIGTERM, [](int singal)
        {
            g_condVarEnd.notify_one();
        });

    auto commandLine = CommandLineParser(argc, argv);


    auto jmsConfigDir = getJMSConfigsDir(commandLine);
    if (!jmsConfigDir.has_value() || !runJMSForAllConfigs(jmsConfigDir.value()))
    {
        LOG_ERR << "Failed to run JMS";
        exit(5);
    }

    auto proxyConfigFile = getProxyConfigFile(commandLine);
    if (!proxyConfigFile.has_value() || !loadProxysFromConfigFile(proxyConfigFile.value()))
    {
        LOG_ERR << "Failed to run Proxys";
        exit(5);
    }

    auto gpsDataDir = getGPSDataDir(commandLine);
    if (!gpsDataDir.has_value())
    {
        LOG_ERR << "GPS input is missing";
        exit(5);
    }
    auto vtConfigFile = getVtConfigFile(commandLine);
    if (!vtConfigFile.has_value())
    {
        LOG_WARN << "VT config file is missing";
        vtConfigFile = "";
    }

    runVTForEachGPS(gpsDataDir.value(), vtConfigFile.value());

    std::mutex mutexEnd;
    std::unique_lock<std::mutex> ulock(mutexEnd);
    g_condVarEnd.wait(ulock);

    closeAllProcesses();
	return 0;
}