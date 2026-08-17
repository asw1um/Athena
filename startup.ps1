if (Get-Command library -ErrorAction SilentlyContinue) {
    $dbPath = "$HOME\.library.db"

    if (-not (Test-Path $dbPath)) {
        Write-Host "[INFO] First-time run detected. Initializing database..." -ForegroundColor Yellow
        library init
    }

    Write-Host "`n--- My Library ---" -ForegroundColor Cyan
    library -ls
    Write-Host ""
}