#pragma once

// 开机启动项的判断依据
#define APPNAME L"FileBackupReminder"

// 询问是否加入开机启动项的文本
#define POWERSTARTMSG L"备份提醒程序没有在开机启动项，是否要加入开机自启？"

// 无法获取工作目录报错文本
#define ERRORWORKDIRMSG L"备份提醒程序遇到异常：无法获取工作目录！"

// 无法初始化配置文件报错文本
#define ERRORPRECONFIGFILEMSG L"备份提醒程序遇到异常：无法初始化配置文件！"

// 无法载入配置文件报错文本
#define ERRORLOADCONFIGFILEMSG L"备份提醒程序遇到异常：无法载入配置文件！"

// 无法压缩文件夹
#define ERRORCOMPRESS L"备份提醒程序遇到异常：无法压缩文件夹！"

// 无法载入配置文件报错文本
#define ERRORWRITECONFIGFILEMSG L"备份提醒程序遇到异常：无法写入配置到配置文件！"