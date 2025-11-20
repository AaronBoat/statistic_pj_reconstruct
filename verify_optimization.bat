@echo off
REM Quick optimization verification script
echo ================================================
echo  Code Optimization Verification
echo ================================================
echo.

echo [1/4] Compiling optimized code...
g++ -std=c++11 -O3 -o test_simple.exe test_simple.cpp MySolution.cpp 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Compilation failed!
    exit /b 1
)
echo SUCCESS: Code compiled successfully
echo.

echo [2/4] Running simple test...
.\test_simple.exe
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Simple test failed!
    exit /b 1
)
echo.

echo [3/4] Checking for small dataset...
if exist ..\data_o\data_o\sift_small (
    echo Found small dataset, running performance test...
    g++ -std=c++11 -O3 -o test_solution.exe test_solution.cpp MySolution.cpp
    .\test_solution.exe ..\data_o\data_o\sift_small
) else (
    echo Small dataset not found, skipping performance test
    echo You can create it with: python create_small_dataset.py
)
echo.

echo [4/4] Summary
echo ================================================
echo Optimizations applied:
echo   ✓ Distance calculation - Loop unrolling (4-way)
echo   ✓ Neighbor pruning - Delayed + partial sort
echo   ✓ ef_search increased - 300 → 400
echo.
echo Expected improvements:
echo   • Build time: -30%% to -40%%
echo   • Recall (GLOVE): +7%% to +10%%
echo   • Query time: similar or slightly slower
echo ================================================
echo.
echo To run full tests:
echo   .\test_solution.exe ..\data_o\data_o\sift
echo   .\test_solution.exe ..\data_o\data_o\glove
echo.
