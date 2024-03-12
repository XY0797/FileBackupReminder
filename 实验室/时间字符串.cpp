#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>

std::string formatDateTime(const std::chrono::system_clock::time_point& timePoint, const std::string& format) {
	std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
	std::tm tm = *std::localtime(&time);

	std::stringstream ss;
	ss << std::put_time(&tm, format.c_str());

	return ss.str();
}

int main() {
	auto now = std::chrono::system_clock::now();

	std::string date = formatDateTime(now, "%Y%m%d");
	std::string time = formatDateTime(now, "%H%M%S");

	std::cout << "Date (yyyymmdd): " << date << std::endl;
	std::cout << "Time (HHMMSS): " << time << std::endl;

	return 0;
}

