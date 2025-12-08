@echo off
REM Quick Grid Search - Test key parameter combinations
echo ========================================
echo   Quick Grid Search - GLOVE
echo ========================================
echo.
echo Compiling...
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp grid_search_glove.cpp MySolution.cpp -o grid_search_quick.exe 2>nul

if %ERRORLEVEL% EQU 0 (
    echo Starting quick grid search...
    echo.
    .\grid_search_quick.exe
) else (
    echo Compilation failed!
)
