@echo off
reg delete "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run" /v FileBackupReminder /f
echo.
if %errorlevel% equ 0 (
    echo ע������ѳɹ�ɾ����
) else (
    echo ɾ��ע�����ʱ������������Ȩ�޻�ȷ�ϸ�ע������Ƿ���ڡ�
)
pause