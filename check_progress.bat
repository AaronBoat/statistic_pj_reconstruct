@echo off
echo Checking test progress...
echo.
if exist sift_test_optimized.txt (
    echo === SIFT Test Progress ===
    tail -n 20 sift_test_optimized.txt
) else (
    echo No SIFT test output found
)
echo.
if exist glove_test_optimized.txt (
    echo === GLOVE Test Progress ===
    tail -n 20 glove_test_optimized.txt
) else (
    echo No GLOVE test output found
)
