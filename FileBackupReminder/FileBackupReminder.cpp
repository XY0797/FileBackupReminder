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
backup_folder=D:\backup

# 压缩后保存路径(路径结尾必须含\)
# %0表示桌面
backup_7z_folder=%0

# 压缩后保存文件名(不含后缀)
# %0表示yyyymmdd的日期，%1表示HHMMSS的时分秒
backup_7z_name=backup_%0%1

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
		CurrentDir = modulePath.substr(0, pos);
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
		std::wstring tmp = pszPath;
		DesktopPath = wstr2astr(tmp);
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
	std::wstring inifilenameW = CurrentDir + L"\\setting.ini";
	if (!willExit && preproceConfigFiles(inifilenameW)) {
		// 解析配置
		std::string inifilenameA = wstr2astr(inifilenameW);
		inifile::IniFile ini;
		if (ini.Load(inifilenameA) != RET_OK) {
			// 输出错误信息
			MessageBox(NULL, ERRORLOADCONFIGFILEMSG, L"错误", MB_ICONERROR | MB_SETFOREGROUND);
			willExit = true;
			PostQuitMessage(0); // 退出程序
		}
		else {
			// 读取配置内容：

			// 上一次提醒时间
			ini.GetIntValueOrDefault("TimeSetting", "last_time", &last_time, 0);

			// 提醒间隔时间，单位为天
			ini.GetIntValueOrDefault("TimeSetting", "remind_interval", &remind_interval, 0);

			// 备份文件夹路径
			ini.GetStringValueOrDefault("PathSetting", "backup_folder", &backup_folder, R"(D:\backup)");

			// 压缩后保存路径(路径结尾必须含\)
			// %0表示桌面
			ini.GetStringValueOrDefault("PathSetting", "backup_7z_folder", &backup_7z_folder, R"(%0)");
			std::regex pattern0(R"(%0)");
			backup_7z_folder = std::regex_replace(backup_7z_folder, pattern0, DesktopPath + "\\");

			// 压缩后保存文件名(不含后缀)
			// %0表示yyyymmdd的日期，%1表示HHMMSS的时分秒
			ini.GetStringValueOrDefault("PathSetting", "backup_7z_name", &backup_7z_name, R"(backup_%0%1)");

			// 提醒窗口标题
			ini.GetStringValueOrDefault("MSGSetting", "remind_title", &remind_titleA, R"(备份提醒)");
			remind_titleW = astr2wstr(remind_titleA);

			// 提醒窗口内容
			// %0表示压缩文件的文件名(含后缀)
			ini.GetStringValueOrDefault("MSGSetting", "remind_content", &remind_contentA, R"(请备份桌面的\n%0\n到网盘)");
			std::regex patternn(R"(\\n)");
			remind_contentA = std::regex_replace(remind_contentA, patternn, "\r\n");
			//remind_contentW = astr2wstr(remind_contentA);
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
	HWND hStatic_notification = CreateWindowEx(0, L"STATIC", L"加载中...",
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

	// 修改hStatic_notification控件显示的文本内容
	std::wstring newText = L"请备份桌面的\nXXXX_20240311.7z\n到网盘";
	SetWindowText(hStatic_notification, newText.c_str());

	// 窗口设置为显示
	ShowWindow(hwnd, SW_SHOW);

	// 窗口消息循环
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}