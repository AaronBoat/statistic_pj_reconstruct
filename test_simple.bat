@echo off
echo Compiling simple test...
g++ -o test_simple.exe test_simple.cpp MySolution.cpp -std=c++11 -O3 -Wall
if %ERRORLEVEL% EQU 0 (
    echo Compilation successful!
    echo.
    echo Running simple test...
    test_simple.exe
) else (
    echo Compilation failed!
)
pause
