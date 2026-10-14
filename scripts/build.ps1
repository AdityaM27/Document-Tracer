# Build script for Document Tracer C++ application and Frontend
Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "     DOCUMENT TRACER: Building C++ Core & Frontend        " -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

# 1. Build C++ Targets with CMake and Ninja
Write-Host "`n[1/3] Configuring C++ build with CMake & Ninja..." -ForegroundColor Yellow
cmake -B build -G Ninja
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed." -ForegroundColor Red
    exit 1
}

Write-Host "`n[2/3] Compiling C++ Executables..." -ForegroundColor Yellow
cmake --build build
if ($LASTEXITCODE -ne 0) {
    Write-Host "C++ compilation failed." -ForegroundColor Red
    exit 1
}

# 2. Build Frontend
Write-Host "`n[3/3] Building React Frontend..." -ForegroundColor Yellow
Push-Location frontend
npm run build
Pop-Location

Write-Host "`n==========================================================" -ForegroundColor Green
Write-Host " Build Completed Successfully!" -ForegroundColor Green
Write-Host " Binaries generated in .\build\" -ForegroundColor Green
Write-Host "   - docutrace_cli.exe" -ForegroundColor Green
Write-Host "   - docutrace_server.exe" -ForegroundColor Green
Write-Host "   - docutrace_tests.exe" -ForegroundColor Green
Write-Host "   - docutrace_benchmark.exe" -ForegroundColor Green
Write-Host "==========================================================" -ForegroundColor Green
