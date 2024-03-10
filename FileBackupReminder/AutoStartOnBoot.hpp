#pragma once

/**
 * 获取程序是否在开机启动项/设置开机启动项
 *
 * by XY0797
 * 2024.3.3
 */

#include "ProjectConfig.h"
#include <Windows.h>
#include <string>

bool IsAppAutoStartEnabled() {
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		if (RegQueryValueEx(hKey, APPNAME, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
			RegCloseKey(hKey);
			return true;
		}
		RegCloseKey(hKey);
	}
	return false;
}

void AddAppToAutoStart() {
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
		wchar_t path[MAX_PATH + 2];
		GetModuleFileName(NULL, path + 1, MAX_PATH);
		path[0] = L'"';
		int i = 0;
		while (path[++i]);
		path[i] = L'"';
		path[i + 1] = 0;
		// 长度需要包含\0
		i += 2;
		i *= sizeof(wchar_t);
		RegSetValueEx(hKey, APPNAME, 0, REG_SZ, (LPBYTE)path, i);
		RegCloseKey(hKey);
	}
}