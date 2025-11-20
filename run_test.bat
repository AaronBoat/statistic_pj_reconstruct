@echo off
echo ========================================
echo Compiling and Testing
echo ========================================
echo.

cd /d c:\codes\data_pj\reconstruct

echo [1/2] Compiling test_simple.exe...
g++ -o test_simple.exe test_simple.cpp MySolution.cpp -std=c++11 -O3 -Wall
if %ERRORLEVEL% NEQ 0 (
    echo Compilation FAILED!
    pause
    exit /b 1
)
echo Compilation successful!
echo.

echo [2/2] Running test_simple.exe...
test_simple.exe
if %ERRORLEVEL% NEQ 0 (
    echo Test FAILED!
    pause
    exit /b 1
)

echo.
echo ========================================
echo All tests PASSED!
echo ========================================
pause
