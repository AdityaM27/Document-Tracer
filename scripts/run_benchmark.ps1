# Run Document Tracer Multithreading & Accuracy Benchmark Suite
Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " Running Document Tracer Concurrency & Accuracy Benchmark Suite  " -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

if (-not (Test-Path ".\build\docutrace_benchmark.exe")) {
    Write-Host "Building benchmark..." -ForegroundColor Yellow
    cmake --build build
}

.\build\docutrace_benchmark.exe
