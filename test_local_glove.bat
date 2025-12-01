@echo off
REM GLOVE Dataset Test Script (Windows Batch)
REM Dataset: 1,192,514 vectors, 100 dimensions
REM Target: Recall@10 >= 98%, Build time < 30 min

echo ========================================
echo Compiling MySolution with test program
echo ========================================
g++ -std=c++11 -O3 test_solution.cpp MySolution.cpp -o test_solution.exe
if errorlevel 1 (
    echo Compilation failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Current HNSW Parameters (Auto-detected):
echo ========================================
echo   For GLOVE (100-dim, 1.2M vectors):
echo   M = 24
echo   ef_construction = 200
echo   ef_search = 300
echo.
echo Expected Performance:
echo   - Recall@10: ~98-99%%
echo   - Build time: ~15-20 minutes
echo   - Query time: ~0.5-1.0 ms
echo   - Avg distance computations: ~20000-25000
echo ========================================
echo.
echo Running test on GLOVE dataset...
echo Dataset path: ..\data_o\data_o\glove
echo.
echo This will take approximately 15-25 minutes...
echo ========================================
echo.

test_solution.exe ..\data_o\data_o\glove

echo.
echo ========================================
echo Test completed!
echo ========================================
pause
