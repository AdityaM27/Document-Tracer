# Document Tracer: Automated Multi-Service Startup Orchestrator
# Starts Python AI Microservice (8000), C++ REST API Server (8080), and Vite Frontend (5173)

$ErrorActionPreference = "Continue"

$projectRoot = (Resolve-Path "$PSScriptRoot\..").Path

Write-Host "====================================================================" -ForegroundColor Cyan
Write-Host "      Document Tracer: Document Similarity & AI Analysis System     " -ForegroundColor Cyan
Write-Host "                         One-Click Startup                          " -ForegroundColor Cyan
Write-Host "====================================================================" -ForegroundColor Cyan
Write-Host " Project Directory: $projectRoot`n" -ForegroundColor Gray

# -----------------------------------------------------------------------------
# Helper Functions
# -----------------------------------------------------------------------------
function Test-PortInUse([int]$Port) {
    try {
        $conns = @(Get-NetTCPConnection -LocalPort $Port -State Listen -ErrorAction SilentlyContinue)
        return ($conns.Count -gt 0)
    } catch {
        return $false
    }
}

function Get-PortProcess([int]$Port) {
    try {
        $conn = Get-NetTCPConnection -LocalPort $Port -State Listen -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($conn) {
            return Get-Process -Id $conn.OwningProcess -ErrorAction SilentlyContinue
        }
    } catch {}
    return $null
}

function Test-AIHealth() {
    try {
        $res = Invoke-RestMethod -Uri "http://127.0.0.1:8000/health" -Method Get -TimeoutSec 2 -ErrorAction Stop
        if ($res.status -eq "ok") { return $res }
    } catch {}
    return $null
}

function Test-CppHealth() {
    try {
        $res = Invoke-RestMethod -Uri "http://127.0.0.1:8080/api/health" -Method Get -TimeoutSec 2 -ErrorAction Stop
        if ($res.status -eq "ok") { return $res }
    } catch {}
    return $null
}

# -----------------------------------------------------------------------------
# 1. Environment & Dependency Checks
# -----------------------------------------------------------------------------
$pythonExe = "$projectRoot\.venv\Scripts\python.exe"
if (Test-Path $pythonExe) {
    Write-Host "[Environment] Dedicated virtualenv detected (.venv)" -ForegroundColor Green
} else {
    $pythonExe = "python"
    Write-Host "[Environment] WARNING: .venv not found. Falling back to system python." -ForegroundColor Yellow
}

# -----------------------------------------------------------------------------
# 2. C++ Backend Binary Verification & Compilation
# -----------------------------------------------------------------------------
$serverExe = "$projectRoot\build\docutrace_server.exe"
if (-not (Test-Path $serverExe)) {
    Write-Host "`n[Build] C++ server binary not found at build\docutrace_server.exe." -ForegroundColor Yellow
    Write-Host "[Build] Compiling C++ Backend REST Server with CMake..." -ForegroundColor Yellow
    
    Push-Location $projectRoot
    cmake -B build -G Ninja
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[Build ERROR] CMake configuration failed." -ForegroundColor Red
        Pop-Location
        exit 1
    }
    cmake --build build --target docutrace_server
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[Build ERROR] C++ server compilation failed." -ForegroundColor Red
        Pop-Location
        exit 1
    }
    Pop-Location
    Write-Host "[Build] docutrace_server.exe compiled successfully!" -ForegroundColor Green
} else {
    Write-Host "[Build] C++ server binary ready: build\docutrace_server.exe" -ForegroundColor Green
}

# -----------------------------------------------------------------------------
# 3. Service 1: Python Flask AI Classification Microservice (Port 8000)
# -----------------------------------------------------------------------------
Write-Host "`n--------------------------------------------------------------------" -ForegroundColor DarkGray
Write-Host "[1/3] Checking AI Classification Microservice (Port 8000)..." -ForegroundColor White

$aiHealth = Test-AIHealth
if ($aiHealth) {
    Write-Host "  -> [ALREADY RUNNING] AI Microservice is online on port 8000." -ForegroundColor Green
    Write-Host "     Model: $($aiHealth.model) | Mode: $($aiHealth.mode) | Pretrained: $($aiHealth.hf_loaded)" -ForegroundColor Gray
} elseif (Test-PortInUse 8000) {
    $proc = Get-PortProcess 8000
    Write-Host "  -> [PORT CONFLICT] Port 8000 is occupied by '$($proc.ProcessName)' (PID $($proc.Id)), but did not respond to /health." -ForegroundColor Red
    Write-Host "     Please stop the conflicting process or run stop.bat before launching." -ForegroundColor Yellow
} else {
    Write-Host "  -> Launching AI Microservice in separate terminal..." -ForegroundColor Yellow
    $aiCmd = "title Document Tracer AI Service (Port 8000) && cd /d `"$projectRoot`" && `"$pythonExe`" ai_service/app.py"
    Start-Process cmd.exe -ArgumentList "/k", $aiCmd
    Write-Host "  -> AI Service started." -ForegroundColor Green
}

# -----------------------------------------------------------------------------
# 4. Service 2: C++ REST API Server (Port 8080)
# -----------------------------------------------------------------------------
Write-Host "`n--------------------------------------------------------------------" -ForegroundColor DarkGray
Write-Host "[2/3] Checking C++ Backend REST Server (Port 8080)..." -ForegroundColor White

$cppHealth = Test-CppHealth
if ($cppHealth) {
    Write-Host "  -> [ALREADY RUNNING] C++ Server is online on port 8080." -ForegroundColor Green
    Write-Host "     Core: $($cppHealth.cppCore) | Indexed: $($cppHealth.referencesIndexed) documents" -ForegroundColor Gray
} elseif (Test-PortInUse 8080) {
    $proc = Get-PortProcess 8080
    Write-Host "  -> [PORT CONFLICT] Port 8080 is occupied by '$($proc.ProcessName)' (PID $($proc.Id)), but did not respond to /api/health." -ForegroundColor Red
    Write-Host "     Please stop the conflicting process or run stop.bat before launching." -ForegroundColor Yellow
} else {
    Write-Host "  -> Launching C++ REST API Server in separate terminal..." -ForegroundColor Yellow
    $cppCmd = "title Document Tracer C++ Server (Port 8080) && cd /d `"$projectRoot`" && build\docutrace_server.exe --port 8080 --threads 4"
    Start-Process cmd.exe -ArgumentList "/k", $cppCmd
    Write-Host "  -> C++ Backend Server started." -ForegroundColor Green
}

# -----------------------------------------------------------------------------
# 5. Service 3: React + TypeScript + Vite Frontend (Port 5173)
# -----------------------------------------------------------------------------
Write-Host "`n--------------------------------------------------------------------" -ForegroundColor DarkGray
Write-Host "[3/3] Checking React Web Frontend (Port 5173)..." -ForegroundColor White

if (Test-PortInUse 5173) {
    Write-Host "  -> [ALREADY RUNNING] Frontend is already listening on port 5173." -ForegroundColor Green
} else {
    Write-Host "  -> Launching Vite Dev Server in separate terminal..." -ForegroundColor Yellow
    $feCmd = "title Document Tracer Frontend (Port 5173) && cd /d `"$projectRoot\frontend`" && npm run dev"
    Start-Process cmd.exe -ArgumentList "/k", $feCmd
    Write-Host "  -> Frontend Dev Server started." -ForegroundColor Green
}

# -----------------------------------------------------------------------------
# 6. Service Health Polling Loop
# -----------------------------------------------------------------------------
Write-Host "`n--------------------------------------------------------------------" -ForegroundColor DarkGray
Write-Host "Waiting for all services to be fully initialized..." -ForegroundColor White

$maxRetries = 15
$aiReady = $false
$cppReady = $false
$feReady = $false

for ($i = 1; $i -le $maxRetries; $i++) {
    if (-not $aiReady) {
        $h = Test-AIHealth
        if ($h) {
            $aiReady = $true
            $aiModelName = $h.model
            $aiModeName = $h.mode
            $aiLoaded = $h.hf_loaded
        }
    }

    if (-not $cppReady) {
        $c = Test-CppHealth
        if ($c) {
            $cppReady = $true
            $cppReferences = $c.referencesIndexed
        }
    }

    if (-not $feReady) {
        if (Test-PortInUse 5173) {
            $feReady = $true
        }
    }

    if ($aiReady -and $cppReady -and $feReady) {
        break
    }

    Write-Host "  [Attempt $i/$maxRetries] Waiting for endpoints... (AI: $(if ($aiReady) {'OK'} else {'...'}), C++: $(if ($cppReady) {'OK'} else {'...'}), UI: $(if ($feReady) {'OK'} else {'...'}))" -ForegroundColor Gray
    Start-Sleep -Seconds 1
}

# -----------------------------------------------------------------------------
# 7. Final Status Report & Browser Launch
# -----------------------------------------------------------------------------
Write-Host "`n====================================================================" -ForegroundColor Cyan
Write-Host "                Document Tracer System Status                       " -ForegroundColor Cyan
Write-Host "====================================================================" -ForegroundColor Cyan

if ($aiReady) {
    Write-Host " [PASS] Python AI Service:   http://127.0.0.1:8000/health" -ForegroundColor Green
    Write-Host "        Model: $aiModelName | Mode: $aiModeName | Pretrained: $aiLoaded" -ForegroundColor Gray
} else {
    Write-Host " [WARN] Python AI Service:   Did not respond within $maxRetries seconds." -ForegroundColor Yellow
    Write-Host "        (Check the 'Document Tracer AI Service' window for loading details)" -ForegroundColor Gray
}

if ($cppReady) {
    Write-Host " [PASS] C++ Backend Server:  http://127.0.0.1:8080/api/health" -ForegroundColor Green
    Write-Host "        Indexed Reference Documents: $cppReferences" -ForegroundColor Gray
} else {
    Write-Host " [WARN] C++ Backend Server:  Did not respond within $maxRetries seconds." -ForegroundColor Yellow
    Write-Host "        (Check the 'Document Tracer C++ Server' window for details)" -ForegroundColor Gray
}

if ($feReady) {
    Write-Host " [PASS] Web Dashboard:       http://localhost:5173" -ForegroundColor Green
} else {
    Write-Host " [WARN] Web Dashboard:       Port 5173 not yet listening." -ForegroundColor Yellow
}

Write-Host "`n Opening http://localhost:5173 in default browser..." -ForegroundColor Cyan
Start-Process "http://localhost:5173"

Write-Host "`n====================================================================" -ForegroundColor Cyan
Write-Host " All Document Tracer components are running in separate terminal windows. " -ForegroundColor Cyan
Write-Host " To safely stop all services, double-click: stop.bat                " -ForegroundColor Yellow
Write-Host "====================================================================`n" -ForegroundColor Cyan
