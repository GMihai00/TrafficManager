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
#include <random>

#include "utile/Logger.hpp"
#include "utile/GeoCoordinate.hpp"
#include "utile/ConfigHelpers.hpp"
#include "utile/CommandLineParser.hpp"
#include "utile/TypeConverters.hpp"

#include "utile/SignalHandler.hpp"

#include "utile/GeoCoordinate.hpp"

using namespace common::utile;

LOGGER("TEST-MAIN");

std::vector<PROCESS_INFORMATION> g_runningProcesses;
std::filesystem::path g_video_path;
std::thread g_vtSpawnerThread;
std::mutex g_mutexUpdate;
bool g_shouldStop = false;

int getRandomNumberWithinRange(int min, int max)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> dist(min, max);

    return dist(gen);
}

std::filesystem::path getWorkingDirectory()
{
    wchar_t pBuf[MAX_PATH];
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

    std::wstring to_wstring() const
    {
        std::wstringstream wss;
        wss << L"Executable: " << m_exe << L"\n";
        wss << L"Arguments: ";
        for (const auto& arg : m_arguments)
        {
            wss << arg << L" ";
        }
        return wss.str();
    }
};

std::string GetLastErrorAsString()
{
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string();
    }

    LPSTR messageBuffer = nullptr;

    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    LocalFree(messageBuffer);

    return message;
}


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

    if (!CreateProcess(NULL,
        LPWSTR(command.c_str()),
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &si,
        &pi)
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

        WaitForSingleObject(pi.hProcess, 1000);

        if (!TerminateProcess(pi.hProcess, 0)) {
            std::cerr << "Error terminating process: " << GetLastErrorAsString() << std::endl;
        }


        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}


void closeLastProcess()
{
    const auto& pi = *g_runningProcesses.rbegin().base();

    HANDLE processHandle = NULL;
    processHandle = OpenProcess(PROCESS_TERMINATE | PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);
    if (processHandle == NULL) {
        std::cerr << "Error opening process: " << GetLastErrorAsString() << std::endl;
        return;
    }

    auto waitingTime = getRandomNumberWithinRange(1000, 3000);

    WaitForSingleObject(pi.hProcess, waitingTime);

    if (!TerminateProcess(pi.hProcess, 0)) {
        std::cerr << "Error terminating process: " << GetLastErrorAsString() << std::endl;
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

// taken from jms file
bool tryToRun4TOsForEachJMS(const model::JMSConfig& config)
{
    std::vector<command_line> commandsToBeRan;

    for (const auto& lane_keyword_pair : config.laneToKeyword)
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

    }

    return std::all_of(commandsToBeRan.begin(), commandsToBeRan.end(), [](const auto& comand) { return createProcessFromSameDirectory(comand); });
}

bool runProxy(const std::filesystem::path& configPath)
{
    command_line cmd;

    cmd.m_exe = L"Proxy.exe";
    cmd.m_arguments.push_back(L"-p_conf");
    cmd.m_arguments.push_back(configPath.wstring());

    return createProcessFromSameDirectory(cmd);
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
            cmd.m_arguments.push_back(L"--display");
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

bool startSpawningVTsRandomly(const std::filesystem::path& gpsDataDir, const std::filesystem::path& configPath)
{
    std::vector<command_line> commandsToBeRan;

    std::vector<std::filesystem::path> configFiles;

    std::error_code ec;
    auto dir_iter = std::filesystem::recursive_directory_iterator(gpsDataDir, ec);

    if (ec)
    {
        LOG_ERR << ec.message();
        return false;
    }

    for (const auto& entry : dir_iter)
    {
        if (std::filesystem::is_regular_file(entry, ec))
        {
            if (!ec)
                configFiles.push_back(entry.path());
        }
    }

    if (configFiles.empty())
        return false;

    g_vtSpawnerThread = std::thread([configFiles, configPath]()
        {
            while (!g_shouldStop)
            {
                auto poz = getRandomNumberWithinRange(0, configFiles.size() - 1);


                std::scoped_lock lock(g_mutexUpdate);
                if (poz >= configFiles.size())
                    continue;

                command_line cmd;
                cmd.m_exe = L"VehicleTracker.exe";
                cmd.m_arguments.push_back(L"-gps_f");
                cmd.m_arguments.push_back(configFiles[poz].wstring());
                cmd.m_arguments.push_back(L"-conf");
                cmd.m_arguments.push_back(configPath.wstring());

                if (createProcessFromSameDirectory(cmd))
                {
                    closeLastProcess();
                    g_runningProcesses.pop_back();
                }
                else
                {
                    LOG_WARN << "Failed to start process, cmd= " << utf16_to_utf8(cmd.to_wstring());
                }
            }

        });

    return true;
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
    auto commandLine = CommandLineParser(argc, argv);

    SignalHandler sigHandler{};
    sigHandler.setAction(SIGINT, [](int /*singal*/)
        {
            LOG_ERR << "SIGINT RECIEVED";
            closeAllProcesses();
        });
    sigHandler.setAction(SIGTERM, [](int /*singal*/)
        {
            LOG_ERR << "SIGTERM RECIEVED";
            closeAllProcesses();
        });

    g_video_path = std::filesystem::path(L"..\\resources\\TestData\\CarTestVideo2.mp4");

    auto jmsConfigDir = getJMSConfigsDir(commandLine);
    if (!jmsConfigDir.has_value() || !runJMSForAllConfigs(jmsConfigDir.value()))
    {
        LOG_ERR << "Failed to run JMS";
        closeAllProcesses();
        return 5;
    }

    auto proxyConfigFile = getProxyConfigFile(commandLine);
    if (!proxyConfigFile.has_value() || !runProxy(proxyConfigFile.value()))
    {
        LOG_ERR << "Failed to run Proxy";
        closeAllProcesses();
        return 5;
    }

    // data generated with https://www.nmeagen.org/, junction resembles "Podul Mihai Viteazu Timisoara, Timis"
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
  
    if (!startSpawningVTsRandomly(gpsDataDir.value(), vtConfigFile.value()))
    {
        LOG_WARN << "Failed to start VTs";
    }

    std::cout << "Press any key to stop\n";
    char input;
    std::cin >> input;

    closeAllProcesses();

    g_shouldStop = true;

    if (g_vtSpawnerThread.joinable())
        g_vtSpawnerThread.join();

	return 0;
}