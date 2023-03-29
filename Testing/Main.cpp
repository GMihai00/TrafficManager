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
#include "utile/TypeConverters.hpp"


using namespace common::utile;

LOGGER("TEST-MAIN");

std::condition_variable g_condVarEnd;
std::vector<PROCESS_INFORMATION> g_runningProcesses;
std::filesystem::path g_video_path;
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

std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

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

    auto command = L"\"" + workingDir.wstring() + L"\"";
    for (const auto& arg : cmd.m_arguments)
    {
        command += L" \"" + arg + L"\"";
    }

    std::wcout << L"[INFO] Attempting to run: " << command << L"\n";
    // Start the child process. 
    if (!CreateProcess(NULL,   // No module name (use command line)
        LPWSTR(command.c_str()),   // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {

        LOG_ERR << "CreateProcess failed" << GetLastErrorAsString();
        return false;
    }

    g_runningProcesses.push_back(pi);
    return true;
}

void closeAllProcesses()
{
    for (const auto& pi : g_runningProcesses)
    {
        HANDLE processHandle = NULL;
        processHandle = OpenProcess(PROCESS_TERMINATE | PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);
        if (processHandle == NULL) {
            std::cerr << "Error opening process: " << GetLastErrorAsString() << std::endl;
            continue;
        }
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, 1000);

        if (!TerminateProcess(pi.hProcess, 0)) {
            std::cerr << "Error terminating process: " << GetLastErrorAsString() << std::endl;
        }


        // Close process and thread handles. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

// taken from jms file
bool tryToRun4TOsForEachJMS(const model::JMSConfig& config)
{
    std::vector<command_line> commandsToBeRan;

    // runnning for same enpoint this is why it was failing ffs
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
        cmd.m_arguments.push_back(L"-vp");
        cmd.m_arguments.push_back(g_video_path.wstring());
        commandsToBeRan.push_back(cmd);
        

        // just 1 camera so unselss i have 4 cameras connected can't really test it out
        break;
    }

    return std::all_of(commandsToBeRan.begin(), commandsToBeRan.end(), [](const auto& comand) { return createProcessFromSameDirectory(comand); });
}

bool runProxy(const model::proxy_config_data& config)
{
    command_line cmd;

    cmd.m_exe = L"Proxy.exe";
    cmd.m_arguments.push_back(L"-h");
    cmd.m_arguments.push_back(utf8_to_utf16(config.ip));
    cmd.m_arguments.push_back(L"-p");
    cmd.m_arguments.push_back(std::to_wstring(config.port));
    cmd.m_arguments.push_back(L"-bsw");
    std::wstring boundSW = std::to_wstring(config.boundSW.latitude) + L"," + std::to_wstring(config.boundSW.longitude);
    cmd.m_arguments.push_back(boundSW);
    cmd.m_arguments.push_back(L"-bne");
    std::wstring boundNE = std::to_wstring(config.boundNE.latitude) + L"," + std::to_wstring(config.boundNE.longitude);;
    cmd.m_arguments.push_back(boundNE);
    cmd.m_arguments.push_back(L"-s");
    cmd.m_arguments.push_back(utf8_to_utf16(config.dbServer));
    cmd.m_arguments.push_back(L"-u");
    cmd.m_arguments.push_back(utf8_to_utf16(config.dbUsername));
    cmd.m_arguments.push_back(L"-ps");
    cmd.m_arguments.push_back(utf8_to_utf16(config.dbPassword));

    return createProcessFromSameDirectory(cmd);
}

bool loadProxysFromConfigFile(const std::filesystem::path& configPath)
{
    auto configs = loadProxyConfigs(configPath.string());
    
    return std::all_of(configs.begin(), configs.end(), [](const auto& config) { return runProxy(config); });
}

bool runJMSForAllConfigs(const std::filesystem::path& jmsConfigDir)
{
    std::vector<command_line> commandsToBeRan;
    std::vector<model::JMSConfig> configs;
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
                configs.push_back(std::move(config.value()));
            }
        }
    }

    auto rez = std::all_of(commandsToBeRan.begin(), commandsToBeRan.end(), [](const auto& comand) { return createProcessFromSameDirectory(comand); });
    for (const auto& config : configs)
    {
        if (!tryToRun4TOsForEachJMS(config))
        {
            LOG_WARN << "Failed to create TOs for junction";
        }
    }

    return rez;
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

    //hardcoded for now
    g_video_path = std::filesystem::path(L"C:\\Users\\Mihai Gherghinescu\\source\\repos\\TrafficManager\\resources\\TestData\\CarTestVideo2.mp4");

    auto jmsConfigDir = getJMSConfigsDir(commandLine);
    if (!jmsConfigDir.has_value() || !runJMSForAllConfigs(jmsConfigDir.value()))
    {
        LOG_ERR << "Failed to run JMS";
        closeAllProcesses();
        return 5;
    }

    auto proxyConfigFile = getProxyConfigFile(commandLine);
    if (!proxyConfigFile.has_value() || !loadProxysFromConfigFile(proxyConfigFile.value()))
    {
        LOG_ERR << "Failed to run Proxys";
        closeAllProcesses();
        return 5;
    }

    auto gpsDataDir = getGPSDataDir(commandLine);
    if (!gpsDataDir.has_value())
    {
        LOG_ERR << "GPS input is missing";
        closeAllProcesses();
        return 5;
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