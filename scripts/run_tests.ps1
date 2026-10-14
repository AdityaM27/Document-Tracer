# Run Document Tracer Automated Test Suite
Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " Running Document Tracer Automated Comprehensive Test Suite  " -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

if (-not (Test-Path ".\build\docutrace_tests.exe")) {
    Write-Host "Building tests..." -ForegroundColor Yellow
    cmake --build build
}

.\build\docutrace_tests.exe
