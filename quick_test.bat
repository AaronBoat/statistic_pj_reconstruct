@echo off
cd /d c:\codes\data_pj\reconstruct
echo Compiling...
g++ -o test_simple.exe test_simple.cpp MySolution.cpp -std=c++11 -O3 -Wall 2> compile_errors.txt
if %ERRORLEVEL% NEQ 0 (
    echo Compilation FAILED! Errors:
    type compile_errors.txt
    del compile_errors.txt
    pause
    exit /b 1
)
del compile_errors.txt
echo Compilation successful!
echo.
echo Running test...
test_simple.exe
pause
