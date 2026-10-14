# Launch Document Tracer Python Pretrained AI Service
param(
    [int]$Port = 8000
)

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " Starting Document Tracer AI Classification Microservice   " -ForegroundColor Cyan
Write-Host " Endpoint: http://127.0.0.1:$Port                          " -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

$env:PORT = $Port

$pythonCmd = "python"
if (Test-Path ".\.venv\Scripts\python.exe") {
    $pythonCmd = ".\.venv\Scripts\python.exe"
    Write-Host "[AI Service] Using virtual environment interpreter: $pythonCmd" -ForegroundColor Green
} else {
    Write-Host "[AI Service] Virtual environment not found. Using system python." -ForegroundColor Yellow
}

& $pythonCmd ai_service/app.py
