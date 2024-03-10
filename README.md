# FileBackupReminder

定时提醒用户备份文件。

会在需要备份时自动压缩对应文件夹到指定路径，然后弹出窗口提示用户备份文件到网盘。

## 程序行为

启动时会检测程序是否在开机启动项，如果不在会询问用户是否加入开机启动项

然后会读取配置文件，如果配置文件不存在会自动生成一个配置文件模板

文件配置内容如下：

```ini
[TimeSetting]
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
remind_content=请备份桌面的\n%0\n到网盘
```

基于上一次提醒的时间，程序会在需要提醒时自动进行压缩打包，然后弹出窗口提醒用户去备份文件。

如果不需要备份，那么程序会直接退出