@echo off
setlocal enabledelayedexpansion
title Document Tracer - Stopping Application...

:: Ensure working directory is the script folder (handles spaces in path safely)
cd /d "%~dp0"

echo ====================================================================
echo         Document Tracer: Intelligent Document Intelligence            
echo                     Stopping Application Stack...                   
echo ====================================================================
echo.

:: Launch the PowerShell shutdown orchestrator with ExecutionPolicy Bypass
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\stop_services.ps1"

if errorlevel 1 (
    echo.
    echo [ERROR] Shutdown encountered an issue. Review the messages above.
    pause
    exit /b 1
)

echo.
echo Closing this shutdown window in 3 seconds...
timeout /t 3 >nul 2>&1 || ping 127.0.0.1 -n 4 >nul
exit /b 0
