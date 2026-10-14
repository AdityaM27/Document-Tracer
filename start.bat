@echo off
setlocal enabledelayedexpansion
title Document Tracer - Starting Application...

:: Ensure working directory is the script folder (handles spaces in path safely)
cd /d "%~dp0"

echo ====================================================================
echo         Document Tracer: Intelligent Document Intelligence            
echo                     Starting Application Stack...                   
echo ====================================================================
echo.

:: Launch the PowerShell startup orchestrator with ExecutionPolicy Bypass
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\start_services.ps1"

if errorlevel 1 (
    echo.
    echo [ERROR] Startup encountered an issue. Review the messages above.
    pause
    exit /b 1
)

:: Automatically dismiss launcher console after 5 seconds or on keypress
echo.
echo Closing this startup launcher window in 5 seconds (services will remain running)...
timeout /t 5 >nul 2>&1 || ping 127.0.0.1 -n 6 >nul
exit /b 0
