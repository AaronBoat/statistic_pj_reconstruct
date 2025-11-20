@echo off
echo Compiling MySolution...
g++ -o test_solution.exe test_solution.cpp MySolution.cpp -std=c++11 -O3 -Wall
if %ERRORLEVEL% EQU 0 (
    echo Compilation successful!
    echo.
    echo Running test...
    test_solution.exe
) else (
    echo Compilation failed!
)
