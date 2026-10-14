# Launch Document Tracer React Web Dashboard (Vite Dev Server)
Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " Starting Document Tracer React Web Dashboard (Vite)       " -ForegroundColor Cyan
Write-Host " Dashboard URL: http://localhost:5173                     " -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

Push-Location frontend
npm run dev
Pop-Location
