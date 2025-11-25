# ========================================
# Local SIFT Dataset Testing Script (PowerShell)
# ========================================

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Compiling MySolution with test program" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green

g++ -std=c++11 -O3 test_solution.cpp MySolution.cpp -o test_solution.exe 2>&1 | Out-Null

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Compilation failed!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "Current HNSW Parameters:" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "  M = 16" -ForegroundColor Cyan
Write-Host "  ef_construction = 200" -ForegroundColor Cyan
Write-Host "  ef_search = 400" -ForegroundColor Cyan
Write-Host ""
Write-Host "Expected Performance:" -ForegroundColor Yellow
Write-Host "  - Recall@10: ~99.4%" -ForegroundColor Cyan
Write-Host "  - Build time: ~18-22 minutes" -ForegroundColor Cyan
Write-Host "  - Query time: ~1-2 ms" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Yellow
Write-Host ""

Write-Host "Running test on SIFT dataset..." -ForegroundColor Green
Write-Host "Dataset path: ..\data_o\data_o\sift" -ForegroundColor Cyan
Write-Host ""
Write-Host "This will take approximately 20-25 minutes..." -ForegroundColor Magenta
Write-Host "========================================" -ForegroundColor Green
Write-Host ""

# Record start time
$startTime = Get-Date

# Run test and save output
.\test_solution.exe ..\data_o\data_o\sift 2>&1 | Tee-Object -FilePath sift_test_result.txt

# Calculate elapsed time
$endTime = Get-Date
$duration = $endTime - $startTime

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Test completed!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "Elapsed time: $($duration.ToString('mm\:ss'))" -ForegroundColor Cyan
Write-Host "Results saved to: sift_test_result.txt" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Green
Write-Host ""

# Extract and display key metrics
Write-Host "Key Metrics Summary:" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow

$content = Get-Content sift_test_result.txt -Raw

if ($content -match "Build time:\s+(\d+)\s+ms") {
    $buildTimeMs = [int]$matches[1]
    $buildTimeSec = [math]::Round($buildTimeMs / 1000, 2)
    $buildTimeMin = [math]::Round($buildTimeMs / 60000, 2)
    Write-Host "  Build Time: $buildTimeSec s ($buildTimeMin min)" -ForegroundColor Cyan
}

if ($content -match "Average search time:\s+([\d.]+)\s+ms") {
    $avgSearchTime = $matches[1]
    Write-Host "  Average Query Time: $avgSearchTime ms" -ForegroundColor Cyan
}

if ($content -match "Recall@10:\s+([\d.]+)") {
    $recall10 = [double]$matches[1]
    $recallPercent = [math]::Round($recall10 * 100, 2)
    
    if ($recall10 -ge 0.98) {
        Write-Host "  Recall@10: $recallPercent% ✓" -ForegroundColor Green
    } else {
        Write-Host "  Recall@10: $recallPercent% ✗ (Target: ≥98%)" -ForegroundColor Red
    }
}

Write-Host "========================================" -ForegroundColor Yellow
Write-Host ""

# Performance evaluation
Write-Host "Performance Evaluation:" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow

$passCount = 0
$totalChecks = 3

if ($content -match "Recall@10:\s+([\d.]+)") {
    $recall10 = [double]$matches[1]
    if ($recall10 -ge 0.98) {
        Write-Host "  [✓] Recall ≥ 98%" -ForegroundColor Green
        $passCount++
    } else {
        Write-Host "  [✗] Recall < 98%" -ForegroundColor Red
    }
}

if ($content -match "Build time:\s+(\d+)\s+ms") {
    $buildTimeMs = [int]$matches[1]
    if ($buildTimeMs -lt 1800000) {  # 30 minutes
        Write-Host "  [✓] Build time < 30 min" -ForegroundColor Green
        $passCount++
    } else {
        Write-Host "  [✗] Build time ≥ 30 min" -ForegroundColor Red
    }
}

if ($content -match "Average search time:\s+([\d.]+)\s+ms") {
    $avgSearchTime = [double]$matches[1]
    if ($avgSearchTime -lt 5.0) {
        Write-Host "  [✓] Query time < 5 ms" -ForegroundColor Green
        $passCount++
    } else {
        Write-Host "  [✗] Query time ≥ 5 ms" -ForegroundColor Red
    }
}

Write-Host ""
if ($passCount -eq $totalChecks) {
    Write-Host "Result: $passCount/$totalChecks checks passed ✓" -ForegroundColor Green
    Write-Host "Ready to submit MySolution.tar!" -ForegroundColor Green
} else {
    Write-Host "Result: $passCount/$totalChecks checks passed" -ForegroundColor Yellow
    Write-Host "Consider parameter tuning for better performance" -ForegroundColor Yellow
}

Write-Host "========================================" -ForegroundColor Yellow
