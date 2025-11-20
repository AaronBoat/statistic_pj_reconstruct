@echo off
echo ============================================
echo Vector Retrieval Project - Setup and Build
echo ============================================
echo.

REM Step 1: Initialize Git
echo [Step 1/4] Initializing Git repository...
if not exist .git (
    git init
    git add .gitignore README.md DEVLOG.md SUMMARY.md QUICKSTART.md AGENT.md
    git add MySolution.h MySolution.cpp
    git add test_solution.cpp test_simple.cpp
    git add Makefile *.bat
    git commit -m "feat: initial HNSW implementation for vector retrieval"
    echo Git repository initialized and committed.
) else (
    echo Git repository already exists.
)
echo.

REM Step 2: Build simple test
echo [Step 2/4] Building simple test...
g++ -o test_simple.exe test_simple.cpp MySolution.cpp -std=c++11 -O3 -Wall
if %ERRORLEVEL% EQU 0 (
    echo Simple test compiled successfully!
) else (
    echo Simple test compilation failed!
    pause
    exit /b 1
)
echo.

REM Step 3: Run simple test
echo [Step 3/4] Running simple test...
test_simple.exe
if %ERRORLEVEL% EQU 0 (
    echo Simple test passed!
) else (
    echo Simple test failed!
    pause
    exit /b 1
)
echo.

REM Step 4: Create package
echo [Step 4/4] Creating submission package...
tar -cvf MySolution.tar MySolution.h MySolution.cpp
if %ERRORLEVEL% EQU 0 (
    echo Package created: MySolution.tar
) else (
    echo Package creation failed!
    pause
    exit /b 1
)
echo.

echo ============================================
echo All steps completed successfully!
echo ============================================
echo.
echo Files ready for submission:
echo   - MySolution.h
echo   - MySolution.cpp
echo   - MySolution.tar
echo.
echo Quick reference:
echo   - See QUICKSTART.md for quick commands
echo   - See README.md for detailed documentation
echo   - See SUMMARY.md for implementation overview
echo.
echo Next steps:
echo   1. Review generated files
echo   2. (Optional) Test with real data: build.bat
echo   3. Upload MySolution.tar to http://10.176.56.208:5000
echo.
echo IMPORTANT: You can only submit once per day!
echo.
pause
