@echo off
REM Monitor SIFT test progress
:loop
cls
echo SIFT Test Progress Monitor
echo ====================================
type sift_test_result.txt
echo.
echo ====================================
echo Press Ctrl+C to stop monitoring
timeout /t 10 >nul
goto loop
