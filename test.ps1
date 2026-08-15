$ErrorActionPreference = "Continue"

$global:passed = 0
$global:failed = 0

function Assert-Test {
    param (
        [string]$name,
        [scriptblock]$commandScript,
        [bool]$expectSuccess = $true,
        [string]$containsText = ""
    )

    Write-Host "`n--------------------------------------------------" -ForegroundColor DarkCyan
    Write-Host "[TEST] $name" -ForegroundColor Cyan

    # Execute command and capture output
    $rawOutput = & $commandScript 2>&1
    $outputStr = ($rawOutput | Out-String).Trim()
    $code = $LASTEXITCODE

    if ($outputStr) {
        Write-Host $outputStr
    } else {
        Write-Host "(No output printed)" -ForegroundColor DarkGray
    }

    $codePassed = ($expectSuccess -and $code -eq 0) -or (-not $expectSuccess -and $code -ne 0)

    $stringPassed = $true
    if ($containsText) {
        $stringPassed = $outputStr.Contains($containsText)
    }

    if ($codePassed -and $stringPassed) {
        Write-Host "[RESULT: PASS]" -ForegroundColor Green
        $global:passed++
    } else {
        Write-Host "[RESULT: FAIL]" -ForegroundColor Red
        if (-not $codePassed) {
            Write-Host "  -> Invalid Exit Code: $code (Expected Success: $expectSuccess)" -ForegroundColor Red
        }
        if (-not $stringPassed) {
            Write-Host "  -> Missing Expected Text: '$containsText'" -ForegroundColor Red
        }
        $global:failed++
    }
}

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "   FULL OUTPUT & STRING CHECK TEST SUITE  " -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

# 0. Clean up existing test database
if (Test-Path "library.db") { Remove-Item "library.db" -Force }

# 1. Standard Pipeline Tests
Assert-Test "Database Init" `
    -commandScript { .\library.exe init }

Assert-Test "Add C Book ($49.99)" `
    -commandScript { .\library.exe add "978-0131103627" "The C Programming Language" "Kernighan & Ritchie" 49.99 "https://isocpp.org/c" "Programming" }

Assert-Test "Add Clean Code Book ($38.50)" `
    -commandScript { .\library.exe add "978-0132350884" "Clean Code" "Robert C. Martin" 38.50 "https://cleancode.org" "Software Engineering" }

Assert-Test "Display Full Library Table" `
    -commandScript { .\library.exe -ls } `
    -containsText "The C Programming Language"

# --- NEW EDIT FEATURE TESTS ---
Assert-Test "Edit Price Only (ID 1 -> $59.99)" `
    -commandScript { .\library.exe edit 1 -p 59.99 }

Assert-Test "Verify Price Update in Table ($59.99)" `
    -commandScript { .\library.exe -ls } `
    -containsText "$59.99"

Assert-Test "Edit Multiple Fields (ID 2 -> Author & Category)" `
    -commandScript { .\library.exe edit 2 -a "Uncle Bob" -c "Best Practices" }

Assert-Test "Verify Author & Category Update (Uncle Bob / Best Practices)" `
    -commandScript { .\library.exe -ls } `
    -containsText "Uncle Bob"

Assert-Test "Reject Edit on Non-Existent ID (ID 99)" `
    -commandScript { .\library.exe edit 99 -p 19.99 } `
    -expectSuccess $false

# --- DELETION TESTS ---
Assert-Test "Delete Book by ID" `
    -commandScript { .\library.exe delete -id 1 }

Assert-Test "Verify Table Output After Deletion" `
    -commandScript { .\library.exe -ls } `
    -containsText "Clean Code"

# 2. Final Summary
$failColor = if ($global:failed -gt 0) { "Red" } else { "Gray" }

Write-Host "`n==========================================" -ForegroundColor Cyan
Write-Host "             TEST SUMMARY                 " -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host " Passed: $global:passed" -ForegroundColor Green
Write-Host " Failed: $global:failed" -ForegroundColor $failColor
Write-Host " Total:  $($global:passed + $global:failed)"

if ($global:failed -eq 0) {
    Write-Host "`nALL TESTS SUCCESSFUL AND VERIFIED!" -ForegroundColor Green
} else {
    Write-Host "`nSOME TESTS FAILED - CHECK OUTPUT ABOVE" -ForegroundColor Red
}