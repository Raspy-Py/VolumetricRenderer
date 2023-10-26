#include "Utils.h"

#include <loguru.hpp>

#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>

void LogsInit()
{
	loguru::g_preamble_date		= false;
	loguru::g_preamble_time		= true;
	loguru::g_preamble_uptime	= false;
	loguru::g_preamble_thread	= false;
	loguru::g_preamble_file		= true;
	loguru::g_preamble_verbose	= true;
	loguru::g_preamble_pipe		= true;
	loguru::g_stderr_verbosity	= loguru::Verbosity_OFF;

	// Hardcoded values I don't care about
	const std::string logsDirectory = "./logs/";
	const std::string logsFile = "latest";
	const std::string logsFileExtension = ".log";
	std::string fullName = logsDirectory + logsFile + logsFileExtension;

	if (std::filesystem::exists(fullName))
	{
		auto now = std::chrono::system_clock::now();
		std::time_t time = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;
		ss	<< logsDirectory 
			<< "backup_" 
			<< std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S")
			<< logsFileExtension;

		std::filesystem::rename(fullName, ss.str());
	}

	loguru::add_file(fullName.c_str(), loguru::Truncate, loguru::Verbosity_INFO);
}

void ReadFile(const std::string& filename, std::vector<char>& buffer)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		Error("Failed to open file: %s", filename.c_str());
	}

	std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	buffer.resize(fileSize);
	if (!file.read(buffer.data(), fileSize))
	{
		Error("Failed to read file: %s", filename.c_str());
	}
}
