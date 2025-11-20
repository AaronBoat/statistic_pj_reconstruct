@echo off
REM Monitor long-running SIFT/GLOVE tests
echo ================================================
echo  Test Progress Monitor
echo ================================================
echo.

:loop
cls
echo ================================================
echo  Test Progress Monitor
echo ================================================
echo Current Time: %TIME%
echo.

REM Check SIFT test
if exist sift_test_optimized.txt (
    echo [SIFT Test]
    for %%A in (sift_test_optimized.txt) do (
        echo File size: %%~zA bytes
        echo Last updated: %%~tA
    )
    echo.
    echo Latest output:
    echo ----------------
    powershell -Command "Get-Content sift_test_optimized.txt -Tail 15"
    echo ----------------
) else (
    echo [SIFT Test] Not started
)

echo.
echo.

REM Check GLOVE test
if exist glove_test_optimized.txt (
    echo [GLOVE Test]
    for %%A in (glove_test_optimized.txt) do (
        echo File size: %%~zA bytes
        echo Last updated: %%~tA
    )
    echo.
    echo Latest output:
    echo ----------------
    powershell -Command "Get-Content glove_test_optimized.txt -Tail 15"
    echo ----------------
) else (
    echo [GLOVE Test] Not started
)

echo.
echo ================================================
echo Press Ctrl+C to stop monitoring
echo Auto-refresh in 30 seconds...
echo ================================================
timeout /t 30 /nobreak > nul
goto loop
