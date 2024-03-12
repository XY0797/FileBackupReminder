#include <iostream>
#include <chrono>

int main() {
	// 转换为时间戳（单位为秒）
	long long ll_timestamp = static_cast<long long>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	std::cout << "ll_timestamp: " << ll_timestamp << std::endl;
	return 0;
}
