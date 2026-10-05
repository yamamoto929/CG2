#include "Log.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <Windows.h>
#include <format>
std::ofstream gLogFile;
void Log(const std::string& message) {
	if (gLogFile.is_open()) {
		gLogFile << message << std::endl;
	}
	OutputDebugStringA(message.c_str());
}

void InitLog() {
	if (gLogFile.is_open()) { gLogFile.close(); }
	std::filesystem::create_directory("logs");

	auto now = std::chrono::system_clock::now();
	auto nowSec = std::chrono::time_point_cast<std::chrono::seconds>(now);
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSec };

	std::string date = std::format("{:%Y%m%d_%H%M%S}", localTime);
	std::string path = "logs/" + date + ".log";

	gLogFile.open(path, std::ios::app);
}
