# Document Tracer: Safe Service Shutdown Script
# Terminates ONLY Document Tracer processes (docutrace_server.exe, python on port 8000, node on port 5173)
# Leaves all unrelated system, Python, and Node processes completely untouched.

$ErrorActionPreference = "Continue"

Write-Host "====================================================================" -ForegroundColor Cyan
Write-Host "      Document Tracer: Safe Service Shutdown Orchestrator           " -ForegroundColor Cyan
Write-Host "====================================================================" -ForegroundColor Cyan

$stoppedCount = 0

# -----------------------------------------------------------------------------
# 1. Stop C++ Server (docutrace_server.exe)
# -----------------------------------------------------------------------------
Write-Host "`n[1/3] Checking C++ Backend Server (docutrace_server)..." -ForegroundColor White
$cppProcs = @(Get-Process -Name docutrace_server -ErrorAction SilentlyContinue)
if ($cppProcs.Count -gt 0) {
    foreach ($p in $cppProcs) {
        Write-Host "  -> Stopping docutrace_server.exe (PID $($p.Id))..." -ForegroundColor Yellow
        Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue
        $stoppedCount++
    }
    Write-Host "  -> [STOPPED] C++ Backend Server terminated." -ForegroundColor Green
} else {
    Write-Host "  -> [INACTIVE] C++ Backend Server is not running." -ForegroundColor Gray
}

# -----------------------------------------------------------------------------
# 2. Stop Python AI Microservice (only if listening on port 8000)
# -----------------------------------------------------------------------------
Write-Host "`n[2/3] Checking AI Classification Microservice (Port 8000)..." -ForegroundColor White
$aiConns = @(Get-NetTCPConnection -LocalPort 8000 -State Listen -ErrorAction SilentlyContinue)
$aiStopped = $false
if ($aiConns.Count -gt 0) {
    foreach ($c in $aiConns) {
        $proc = Get-Process -Id $c.OwningProcess -ErrorAction SilentlyContinue
        if ($proc -and $proc.ProcessName -eq "python") {
            Write-Host "  -> Stopping Python AI Microservice (PID $($proc.Id)) on port 8000..." -ForegroundColor Yellow
            Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
            $aiStopped = $true
            $stoppedCount++
        }
    }
}

if ($aiStopped) {
    Write-Host "  -> [STOPPED] AI Microservice on port 8000 terminated." -ForegroundColor Green
} else {
    Write-Host "  -> [INACTIVE] No Python process listening on port 8000." -ForegroundColor Gray
}

# -----------------------------------------------------------------------------
# 3. Stop React / Vite Frontend (only if listening on port 5173)
# -----------------------------------------------------------------------------
Write-Host "`n[3/3] Checking React Web Frontend (Port 5173)..." -ForegroundColor White
$feConns = @(Get-NetTCPConnection -LocalPort 5173 -State Listen -ErrorAction SilentlyContinue)
$feStopped = $false
if ($feConns.Count -gt 0) {
    foreach ($c in $feConns) {
        $proc = Get-Process -Id $c.OwningProcess -ErrorAction SilentlyContinue
        if ($proc -and ($proc.ProcessName -match "node|cmd")) {
            Write-Host "  -> Stopping Frontend process '$($proc.ProcessName)' (PID $($proc.Id)) on port 5173..." -ForegroundColor Yellow
            Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
            $feStopped = $true
            $stoppedCount++
        }
    }
}

if ($feStopped) {
    Write-Host "  -> [STOPPED] Frontend dev server on port 5173 terminated." -ForegroundColor Green
} else {
    Write-Host "  -> [INACTIVE] No process listening on Frontend port 5173." -ForegroundColor Gray
}

# -----------------------------------------------------------------------------
# 4. Clean up any lingering terminal windows titled "Document Tracer *" or "DocuTrace *"
# -----------------------------------------------------------------------------
try {
    $windows = @(Get-Process -Name cmd, powershell, conhost -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowTitle -like "Document Tracer *" -or $_.MainWindowTitle -like "DocuTrace *" })
    foreach ($w in $windows) {
        Stop-Process -Id $w.Id -Force -ErrorAction SilentlyContinue
    }
} catch {}

Write-Host "`n====================================================================" -ForegroundColor Cyan
Write-Host " Shutdown Complete. Total processes stopped: $stoppedCount" -ForegroundColor Green
Write-Host " All other system, Python, and Node applications were left untouched." -ForegroundColor Gray
Write-Host "====================================================================`n" -ForegroundColor Cyan
