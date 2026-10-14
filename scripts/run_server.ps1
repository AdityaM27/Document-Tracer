param(
    [int]$Port = 8080,
    [int]$Threads = 4
)

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "Starting Document Tracer C++ REST API Server on port $Port" -ForegroundColor Cyan
Write-Host "ThreadPool Workers: $Threads" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

if (-not (Test-Path ".\build\docutrace_server.exe")) {
    Write-Host "docutrace_server.exe not found. Building now..." -ForegroundColor Yellow
    cmake --build build
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed. Please check the CMake errors above." -ForegroundColor Red
        exit 1
    }
}

if (-not (Test-Path ".\build\docutrace_server.exe")) {
    Write-Host "Server executable was not created." -ForegroundColor Red
    exit 1
}

& ".\build\docutrace_server.exe" --port $Port --threads $Threads
