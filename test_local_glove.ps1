# GLOVE Dataset Test Script (PowerShell)
# Dataset: 1,192,514 vectors, 100 dimensions
# Target: Recall@10 >= 98%, Build time < 30 min

Write-Host "`n========================================"
Write-Host "Compiling MySolution with test program"
Write-Host "========================================`n"

g++ -std=c++11 -O3 test_solution.cpp MySolution.cpp -o test_solution.exe
if ($LASTEXITCODE -ne 0) {
    Write-Host "Compilation failed!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================"
Write-Host "Current HNSW Parameters (Auto-detected):"
Write-Host "========================================"
Write-Host "  For GLOVE (100-dim, 1.2M vectors):"
Write-Host "  M = 24"
Write-Host "  ef_construction = 200"
Write-Host "  ef_search = 300"
Write-Host ""
Write-Host "Expected Performance:"
Write-Host "  - Recall@10: ~98-99%"
Write-Host "  - Build time: ~15-20 minutes"
Write-Host "  - Query time: ~0.5-1.0 ms"
Write-Host "  - Avg distance computations: ~20000-25000"
Write-Host "========================================`n"

Write-Host "Running test on GLOVE dataset..."
Write-Host "Dataset path: ..\data_o\data_o\glove`n"

Write-Host "This will take approximately 15-25 minutes..."
Write-Host "========================================`n"

.\test_solution.exe ..\data_o\data_o\glove

Write-Host "`n========================================"
Write-Host "Test completed!"
Write-Host "========================================`n"
