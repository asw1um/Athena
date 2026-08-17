
$ErrorActionPreference = "Stop"

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "   ATHENA - AUTOMATED INSTALLER      " -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

$installDir = "$HOME\.athena\bin"
$exePath = "$installDir\athena.exe"

Write-Host "`n[1/4] Creating installation directory at:" -ForegroundColor Yellow
Write-Host "      $installDir"
if (-not (Test-Path $installDir)) {
    New-Item -ItemType Directory -Force -Path $installDir | Out-Null
}

Write-Host "`n[2/4] Compiling binary with g++..." -ForegroundColor Yellow
g++ -std=c++17 -Wall -Wextra main.cpp db.cpp tableGen.cpp -o $exePath -lsqlite3

if ($LASTEXITCODE -ne 0) {
    Write-Host "`n[ERROR] Compilation failed! Ensure g++ and libsqlite3 are installed." -ForegroundColor Red
    exit 1
}
Write-Host "      Build successful -> $exePath" -ForegroundColor Green

Write-Host "`n[3/4] Updating PATH environment variable..." -ForegroundColor Yellow
$currentPath = [Environment]::GetEnvironmentVariable("Path", "User")

if ($currentPath -split ';' -contains $installDir) {
    Write-Host "      $installDir is already in your User PATH." -ForegroundColor Gray
} else {
    $newPath = "$currentPath;$installDir"
    [Environment]::SetEnvironmentVariable("Path", $newPath, "User")
    $env:Path += ";$installDir"
    Write-Host "      Added $installDir to User PATH." -ForegroundColor Green
}

# 4. Initialize Database
Write-Host "`n[4/4] Initializing database schema..." -ForegroundColor Yellow
& $exePath init

Write-Host "`n==========================================" -ForegroundColor Cyan
Write-Host "      INSTALLATION COMPLETE!              " -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Restart your terminal, then run from ANY directory:"
Write-Host "  athena -init (this part can be neglected if startup terminal display is enabled before running)" -ForegroundColor Yellow
Write-Host "  athena -ls" -ForegroundColor Yellow
Write-Host "  athena add <isbn> <title> <author> <price> <link> <category>" -ForegroundColor Yellow