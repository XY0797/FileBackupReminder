@echo off
reg delete "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run" /v FileBackupReminder /f
echo.
if %errorlevel% equ 0 (
    echo 注册表项已成功删除。
) else (
    echo 删除注册表项时发生错误，请检查权限或确认该注册表项是否存在。
)
pause