#pragma once

// 从Windows头文件中排除极少使用的内容
#define WIN32_LEAN_AND_MEAN
// Windows头文件
#include <windows.h>
#include <shlobj.h>

// 字符串处理
#include <string>
#include <regex>

// 时间处理
#include <chrono>

// 文件流
#include <fstream>

// 开机自启
#include "AutoStartOnBoot.hpp"

// 项目配置
#include "ProjectConfig.h"

// INI解析库
#include "inifile.h"

// 窗口句柄
HWND hwnd;

// 控件ID
#define IDC_FIRST_LABEL 114
#define IDC_NOTIFICATION_LABEL 514

// 是否即将退出
bool willExit;

// 当前目录
std::wstring CurrentDir;

// 桌面路径
std::string DesktopPath;

//-----------配置文件-----------

// 上一次提醒时间
int last_time;

// 提醒间隔时间，单位为天
int remind_interval;

// 备份文件夹路径
std::string backup_folder;

// 压缩后保存路径(路径结尾必须含\)
// %0表示桌面
std::string backup_7z_folder;

// 压缩后保存文件名(不含后缀)
// %0表示yyyymmdd的日期，%1表示HHMMSS的时分秒
std::string backup_7z_name;

// 提醒窗口标题
std::string remind_titleA;
std::wstring remind_titleW;

// 提醒窗口内容
// %0表示压缩文件的文件名(含后缀)
std::string remind_contentA;
std::wstring remind_contentW;

//--------配置文件-----结束-----