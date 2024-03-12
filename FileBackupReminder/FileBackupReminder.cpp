/**
 * 开机时检测是否需要备份，在需要时提醒用户
 *
 * by XY0797
 * 2024.3.10
 */

#include "FileBackupReminder.h"

std::string wstr2astr(const std::wstring& wstr) {
	wchar_t* wideCharBuffer = new wchar_t[wstr.length() + 1];
	wideCharBuffer[wstr.length()] = 0;
	for (size_t i = 0; i < wstr.length(); i++)
	{
		wideCharBuffer[i] = wstr[i];
	}
	int size_needed = WideCharToMultiByte(CP_ACP, 0, wideCharBuffer, -1, NULL, 0, NULL, NULL);
	char* charBuffer = new char[size_needed + 1];
	WideCharToMultiByte(CP_ACP, 0, wideCharBuffer, -1, charBuffer, size_needed, NULL, NULL);
	charBuffer[size_needed] = 0;
	std::string strTo(charBuffer);
	delete[] wideCharBuffer;
	delete[] charBuffer;
	return strTo;
}

std::wstring astr2wstr(const std::string& str) {
	char* charBuffer = new char[str.length() + 1];
	charBuffer[str.length()] = 0;
	for (size_t i = 0; i < str.length(); i++)
	{
		charBuffer[i] = str[i];
	}
	int wideCharLen = MultiByteToWideChar(CP_ACP, 0, charBuffer, -1, nullptr, 0);
	wchar_t* wideCharBuffer = new wchar_t[wideCharLen + 1];
	MultiByteToWideChar(CP_ACP, 0, charBuffer, -1, wideCharBuffer, wideCharLen);
	wideCharBuffer[wideCharLen] = 0;
	std::wstring tmpwstr(wideCharBuffer);
	delete[] wideCharBuffer;
	delete[] charBuffer;
	return tmpwstr;
}

// 一定要记得释放
wchar_t* astr2cwstr(const std::string& str) {
	char* charBuffer = new char[str.length() + 1];
	charBuffer[str.length()] = 0;
	for (size_t i = 0; i < str.length(); i++)
	{
		charBuffer[i] = str[i];
	}
	int wideCharLen = MultiByteToWideChar(CP_ACP, 0, charBuffer, -1, nullptr, 0);
	wchar_t* wideCharBuffer = new wchar_t[wideCharLen + 1];
	MultiByteToWideChar(CP_ACP, 0, charBuffer, -1, wideCharBuffer, wideCharLen);
	wideCharBuffer[wideCharLen] = 0;
	delete[] charBuffer;
	return wideCharBuffer;
}

int win32system(const std::string& commandstr) {
	wchar_t* szCmdline = astr2cwstr(commandstr);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL,   // 不继承进程句柄
		szCmdline,    // 要执行的命令行
		NULL,           // 不继承进程安全属性
		NULL,           // 不继承线程安全属性
		FALSE,          // 不继承句柄
		0,              // 创建标志，例如CREATE_NEW_CONSOLE用于创建新控制台窗口
		NULL,           // 使用父进程的环境块
		NULL,           // 使用父进程的工作目录
		&si,            // 启动信息
		&pi))           // 进程信息
	{
		delete[] szCmdline;
		return -1;
	}
	delete[] szCmdline;

	// 等待子进程结束
	WaitForSingleObject(pi.hProcess, INFINITE);

	// 获取进程退出代码
	DWORD exitCode;
	GetExitCodeProcess(pi.hProcess, &exitCode);

	// 关闭句柄
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return exitCode;
}

bool compressFolder(const std::string& folderPath, const std::string& outputPath, int compressionLevel, const std::string& password = "")
{
	// 构造7zip命令行参数
	std::string command = "\"" + CurrentDirA + "\\7z.exe\" a -t7z ";
	if (!password.empty()) {
		command += "-p" + password + " ";  // 添加密码参数
	}
	command += "-mmt=on -mx" + std::to_string(compressionLevel) + " ";  // 压缩级别
	command += "\"" + outputPath + "\" \"" + folderPath + "\"";  // 输出文件路径和待压缩文件夹路径

	// 执行系统命令
	int result = win32system(command);

	// 检查命令执行结果
	return(result == 0);
}

std::string formatDateTime(const std::chrono::system_clock::time_point& timePoint, const std::string& format) {
	std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
	std::tm tm;
	localtime_s(&tm, &time);

	std::stringstream ss;
	ss << std::put_time(&tm, format.c_str());

	return ss.str();
}

long long getCurTimeStamp() {
	return static_cast<long long>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

bool preproceConfigFiles(const std::wstring& filename) {
	std::ifstream fin(filename);
	if (fin.good()) {
		fin.close();
	}
	else
	{
		std::ofstream fo(filename);
		if (!fo.good()) {
			// 输出错误信息
			MessageBox(NULL, ERRORPRECONFIGFILEMSG, L"错误", MB_ICONERROR | MB_SETFOREGROUND);
			PostQuitMessage(0); // 退出程序
			return false;
		}
		fo << R"([TimeSetting]
# 上一次提醒时间
last_time=0

# 提醒间隔时间，单位为天
remind_interval=24

[PathSetting]

# 备份文件夹路径
# %0表示程序所在目录(结尾含\)
backup_folder=%0

# 压缩后保存路径(路径结尾必须含\)
# %0表示桌面(结尾含\)
backup_7z_folder=%0

# 压缩后保存文件名(不含后缀)
# %0表示yyyymmdd的日期，%1表示HHMMSS的时分秒
backup_7z_name=backup_%0%1

[7ZSetting]

# 压缩等级，范围为0-9，0表示最快但体积最大，9表示最慢但体积最小
7z_compression_level=5

# 密码，留空表示不加密。密码内不得包含双引号、空格、等于号！
7z_password=

[MSGSetting]

# 提醒窗口标题
remind_title=备份提醒

# 提醒窗口内容
# %0表示压缩文件的文件名(含后缀)
# \n表示换行
remind_content=请备份桌面的\n%0\n到网盘)";
		fo.close();
	}
	return true;
}

void refreshCurDir() {
	// 取运行目录
	wchar_t path[MAX_PATH * 2];
	GetModuleFileName(NULL, path, MAX_PATH * 2);
	std::wstring modulePath(path);
	std::wstring::size_type pos = modulePath.find_last_of(L"\\");
	if (pos != std::wstring::npos)
	{
		CurrentDirW = modulePath.substr(0, pos);
		CurrentDirA = wstr2astr(CurrentDirW);
	}
	else
	{
		// 输出错误信息
		MessageBox(NULL, ERRORWORKDIRMSG, L"错误", MB_ICONERROR | MB_SETFOREGROUND);
		willExit = true;
		PostQuitMessage(0); // 退出程序
	}
	PWSTR pszPath;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &pszPath))) {
		DesktopPathW = pszPath;
		DesktopPathA = wstr2astr(DesktopPathW);
		// 释放内存
		CoTaskMemFree(pszPath);
	}
	else {
		// 输出错误信息
		MessageBox(NULL, ERRORWORKDIRMSG, L"错误", MB_ICONERROR | MB_SETFOREGROUND);
		willExit = true;
		PostQuitMessage(0); // 退出程序
	}
}

// 窗口过程函数
HDC hdcStatic;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CTLCOLORSTATIC:
		hdcStatic = (HDC)wParam;
		SetBkMode(hdcStatic, TRANSPARENT); //透明背景
		return (INT_PTR)GetStockObject(NULL_BRUSH); //无颜色画刷
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

// 入口函数
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	willExit = false;
	// 刷新当前工作目录
	refreshCurDir();

	// 预处理配置文件
	std::wstring inifilenameW = CurrentDirW + L"\\setting.ini";
	if (!willExit && preproceConfigFiles(inifilenameW)) {
		try
		{
			// 解析配置
			std::string inifilenameA = wstr2astr(inifilenameW);
			inifile::IniFile ini;
			if (ini.Load(inifilenameA) != RET_OK) {
				// 输出错误信息
				MessageBox(NULL, ERRORLOADCONFIGFILEMSG, L"错误", MB_ICONERROR | MB_SETFOREGROUND);
				throw std::runtime_error("error");
			}
			else {
				// 读取配置内容：

				// 上一次提醒时间
				std::string timeStr;
				ini.GetStringValueOrDefault("TimeSetting", "last_time", &timeStr, "0");
				last_time = stoll(timeStr);

				// 提醒间隔时间，单位为天
				int remind_interval_day;
				ini.GetIntValueOrDefault("TimeSetting", "remind_interval", &remind_interval_day, 0);
				remind_interval = (long long)remind_interval_day * 86400LL;

				// 备份文件夹路径(路径结尾必须含\)
				// %0表示程序所在目录(结尾含\)
				std::regex pattern0(R"(%0)");
				ini.GetStringValueOrDefault("PathSetting", "backup_folder", &backup_folder, R"(%0)");
				backup_folder = std::regex_replace(backup_folder, pattern0, CurrentDirA + "\\");

				// 压缩后保存路径(路径结尾必须含\)
				// %0表示桌面(结尾含\)
				ini.GetStringValueOrDefault("PathSetting", "backup_7z_folder", &backup_7z_folder, R"(%0)");
				backup_7z_folder = std::regex_replace(backup_7z_folder, pattern0, DesktopPathA + "\\");

				// 压缩后保存文件名(不含后缀)
				// %0表示yyyymmdd的日期，%1表示HHMMSS的时分秒
				ini.GetStringValueOrDefault("PathSetting", "backup_7z_name", &backup_7z_name, R"(backup_%0%1)");

				// 压缩等级，范围为0 - 9，0表示最快但体积最大，9表示最慢但体积最小
				ini.GetIntValueOrDefault("7ZSetting", "7z_compression_level", &the7z_compression_level, 5);
				if (the7z_compression_level < 0) { the7z_compression_level = 0; }
				if (the7z_compression_level > 9) { the7z_compression_level = 9; }

				// 密码，留空表示不加密
				ini.GetStringValueOrDefault("7ZSetting", "7z_password", &the7z_password, "");

				// 提醒窗口标题
				ini.GetStringValueOrDefault("MSGSetting", "remind_title", &remind_titleA, R"(备份提醒)");
				remind_titleW = astr2wstr(remind_titleA);

				// 提醒窗口内容
				// %0表示压缩文件的文件名(含后缀)
				ini.GetStringValueOrDefault("MSGSetting", "remind_content", &remind_contentA, R"(请备份桌面的\n%0\n到网盘)");
				std::regex patternn(R"(\\n)");
				remind_contentA = std::regex_replace(remind_contentA, patternn, "\r\n");

				// 处理last_time为0的情况
				if (last_time < 1) {
					last_time = getCurTimeStamp();
					std::string curtime_str = std::to_string(last_time);
					if (ini.SetStringValue("TimeSetting", "last_time", curtime_str) != RET_OK) {
						MessageBox(NULL, ERRORWRITECONFIGFILEMSG, L"错误", MB_ICONERROR | MB_SETFOREGROUND);
						throw std::runtime_error("error");
					}
					if (ini.Save() != RET_OK) {
						MessageBox(NULL, ERRORWRITECONFIGFILEMSG, L"错误", MB_ICONERROR | MB_SETFOREGROUND);
						throw std::runtime_error("error");
					}
				}
			}
		}
		catch (const std::exception&)
		{
			willExit = true;
			PostQuitMessage(0); // 退出程序
		}
	}

	// 注册窗口类
	const wchar_t CLASS_NAME[] = L"Info Window Class";
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);

	// 创建窗口
	hwnd = CreateWindowEx(WS_EX_TOPMOST, CLASS_NAME, remind_titleW.c_str(), WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 416, 306, NULL, NULL, hInstance, NULL);

	if (hwnd == NULL) {
		return 0;
	}

	// 获取纯白画刷
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // RGB值表示白色

	// 画刷获取失败
	if (hBrush == NULL) {
		DestroyWindow(hwnd);
		return 0;
	}

	// 设置窗口背景色为白色
	SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);

	// 创建第一个标签控件（粗体、居中、不允许换行）
	HWND hStatic1 = CreateWindowEx(0, L"STATIC", L"需要备份文件了",
		WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE | SS_NOTIFY | ES_AUTOHSCROLL,
		8, 8, 384, 56, hwnd, (HMENU)IDC_FIRST_LABEL, hInstance, NULL);

	if (hStatic1)
	{
		// 设置字体为黑体、粗体
		HFONT hFont1 = CreateFont(38, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"黑体");
		if (hFont1)
		{
			SendMessage(hStatic1, WM_SETFONT, (WPARAM)hFont1, TRUE);
		}
	}

	// 创建第二个标签控件（常规、左对齐、允许换行）
	HWND hStatic_notification = CreateWindowEx(0, L"STATIC", L"加载信息中...",
		WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL,
		8, 72, 384, 180, hwnd, (HMENU)IDC_NOTIFICATION_LABEL, hInstance, NULL);

	if (hStatic_notification)
	{
		// 设置字体为黑体
		HFONT hFont2 = CreateFont(26, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"黑体");
		if (hFont2)
		{
			SendMessage(hStatic_notification, WM_SETFONT, (WPARAM)hFont2, TRUE);
		}
	}

	// 加载不出来就直接噶窗口
	if (hStatic1 == NULL || hStatic_notification == NULL) {
		DestroyWindow(hwnd);
		return 0;
	}

	// 窗口设置为隐藏
	ShowWindow(hwnd, SW_HIDE);

	// 开机自启检测
	if (!IsAppAutoStartEnabled()) {
		int result = MessageBox(NULL, POWERSTARTMSG, L"询问", MB_YESNO | MB_ICONQUESTION);
		if (result == IDYES) {
			AddAppToAutoStart();
			MessageBox(NULL, L"已添加到开机自启", L"提示", MB_ICONINFORMATION);
		}
	}

	long long cur_time = getCurTimeStamp();
	if (!willExit) {
		try
		{
			if (cur_time - last_time > remind_interval) {
				// 初始化输出文件名
				std::string outfilename = backup_7z_name + ".7z";
				std::string outfilepath = backup_7z_folder;

				// 获取时间信息
				auto nowTime = std::chrono::system_clock::now();
				std::string date_str = formatDateTime(nowTime, "%Y%m%d");
				std::string time_str = formatDateTime(nowTime, "%H%M%S");

				// 生成输出文件名
				std::regex pattern0(R"(%0)");
				std::regex pattern1(R"(%1)");
				outfilename = std::regex_replace(outfilename, pattern0, date_str);
				outfilename = std::regex_replace(outfilename, pattern1, time_str);
				outfilepath += outfilename;

				// 压缩
				if (compressFolder(backup_folder, outfilepath, the7z_compression_level, the7z_password)) {
					// 压缩成功，修改配置文件的上一次提醒时间
					std::string inifilenameA = wstr2astr(inifilenameW);
					inifile::IniFile ini;
					if (ini.Load(inifilenameA) == RET_OK) {
						std::string curtime_str = std::to_string(cur_time);
						if (ini.SetStringValue("TimeSetting", "last_time", curtime_str) != RET_OK) {
							MessageBox(NULL, ERRORWRITECONFIGFILEMSG, L"错误", MB_ICONERROR | MB_SETFOREGROUND);
							throw std::runtime_error("error");
						}
						if (ini.Save() != RET_OK) {
							MessageBox(NULL, ERRORWRITECONFIGFILEMSG, L"错误", MB_ICONERROR | MB_SETFOREGROUND);
							throw std::runtime_error("error");
						}
						// 生成提示文本
						remind_contentA = std::regex_replace(remind_contentA, pattern0, outfilename);
						remind_contentW = astr2wstr(remind_contentA);

						// 修改hStatic_notification控件显示的文本内容
						SetWindowText(hStatic_notification, remind_contentW.c_str());

						// 窗口设置为显示
						ShowWindow(hwnd, SW_SHOW);
					}
					else {
						// 输出错误信息
						MessageBox(NULL, ERRORLOADCONFIGFILEMSG, L"错误", MB_ICONERROR | MB_SETFOREGROUND);
						throw std::runtime_error("error");
					}
				}
				else {
					// 输出错误信息
					MessageBox(NULL, ERRORCOMPRESS, L"错误", MB_ICONERROR | MB_SETFOREGROUND);
					throw std::runtime_error("error");
				}
			}
			else {
				throw std::runtime_error("ok");
			}
		}
		catch (const std::exception&)
		{
			willExit = true;
			PostQuitMessage(0); // 退出程序
		}
	}

	// 窗口消息循环
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}