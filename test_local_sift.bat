@echo off
REM ========================================
REM Local SIFT Dataset Testing Script
REM ========================================

echo ========================================
echo Compiling MySolution with test program
echo ========================================
g++ -std=c++11 -O3 test_solution.cpp MySolution.cpp -o test_solution.exe
if errorlevel 1 (
    echo ERROR: Compilation failed!
    exit /b 1
)

echo.
echo ========================================
echo Current HNSW Parameters:
echo ========================================
echo   M = 16
echo   ef_construction = 100
echo   ef_search = 200
echo.
echo Expected Performance:
echo   - Recall@10: ~98%%+
echo   - Build time: ~10-15 minutes
echo   - Query time: ~0.3-0.5 ms
echo   - Avg distance computations: ~2500-3500
echo ========================================
echo.

echo Running test on SIFT dataset...
echo Dataset path: ..\data_o\data_o\sift
echo.
echo This will take approximately 20-25 minutes...
echo Press Ctrl+C to cancel if needed
echo ========================================
echo.

REM Run test and display output in real-time
test_solution.exe ..\data_o\data_o\sift

echo.
echo ========================================
echo Test completed!
echo ========================================
echo.
echo Check the following metrics:
echo   1. Recall@10 should be ^>= 0.98
echo   2. Build time should be ^< 30 minutes
echo   3. Average query time should be ^< 5 ms
echo ========================================
echo.
echo To save output to file, use PowerShell version:
echo   .\test_local_sift.ps1
echo ========================================
