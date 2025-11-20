@echo off
REM Grid Search for SIFT Parameter Tuning
echo ============================================================
echo  SIFT Parameter Grid Search
echo ============================================================
echo.

echo [Step 1/3] Compiling grid search program...
g++ -std=c++11 -O3 -o grid_search_sift.exe grid_search_sift.cpp MySolution.cpp
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Compilation failed!
    pause
    exit /b 1
)
echo SUCCESS: Compiled successfully
echo.

echo [Step 2/3] Running grid search on SIFT small dataset...
echo This may take 10-20 minutes depending on parameter grid size
echo.
.\grid_search_sift.exe ..\data_o\data_o\sift_small
echo.

echo [Step 3/3] Results saved to:
echo   - sift_grid_search_results.csv (machine-readable)
echo   - sift_grid_search_report.md (human-readable)
echo.

echo ============================================================
echo Grid search completed!
echo ============================================================
echo.
echo To view results:
echo   - Excel/LibreOffice: Open sift_grid_search_results.csv
echo   - Markdown viewer: Open sift_grid_search_report.md
echo   - Terminal: type sift_grid_search_results.csv
echo.
pause
