@echo off
REM Quick comparison test for different parameter configurations
setlocal enabledelayedexpansion

echo ========================================
echo Quick Parameter Comparison Test
echo ========================================
echo.

set DATASET=..\data_o\data_o\glove

REM Create temporary backup
copy MySolution.cpp MySolution.cpp.bak >nul

echo Testing 3 configurations...
echo.

REM Config 1: Current optimized (M=18, ef_c=150, ef_s=2400)
echo [1/3] Config 1: M=18, ef_c=150, ef_s=2400 (Current)
echo ----------------------------------------
.\test_opt.exe %DATASET% 2>&1 | findstr /C:"Build time" /C:"Total search" /C:"Average search" /C:"distance computations" /C:"Recall"
echo.

REM Config 2: Higher M for better recall (M=20, ef_c=160, ef_s=2600)
echo [2/3] Config 2: M=20, ef_c=160, ef_s=2600 (Higher quality)
echo ----------------------------------------
python -c "import re; content=open('MySolution.cpp','r',encoding='utf-8').read(); content=re.sub(r'M = \d+;', 'M = 20;', content); content=re.sub(r'ef_construction = \d+;', 'ef_construction = 160;', content); content=re.sub(r'ef_search = \d+;', 'ef_search = 2600;', content); open('MySolution.cpp','w',encoding='utf-8').write(content)"
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp test_solution.cpp MySolution.cpp -o test_cfg2.exe 2>nul
if exist test_cfg2.exe (
    .\test_cfg2.exe %DATASET% 2>&1 | findstr /C:"Build time" /C:"Total search" /C:"Average search" /C:"distance computations" /C:"Recall"
) else (
    echo Compilation failed
)
echo.

REM Config 3: Lower for speed (M=16, ef_c=140, ef_s=2200)
echo [3/3] Config 3: M=16, ef_c=140, ef_s=2200 (Faster)
echo ----------------------------------------
python -c "import re; content=open('MySolution.cpp','r',encoding='utf-8').read(); content=re.sub(r'M = \d+;', 'M = 16;', content); content=re.sub(r'ef_construction = \d+;', 'ef_construction = 140;', content); content=re.sub(r'ef_search = \d+;', 'ef_search = 2200;', content); open('MySolution.cpp','w',encoding='utf-8').write(content)"
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp test_solution.cpp MySolution.cpp -o test_cfg3.exe 2>nul
if exist test_cfg3.exe (
    .\test_cfg3.exe %DATASET% 2>&1 | findstr /C:"Build time" /C:"Total search" /C:"Average search" /C:"distance computations" /C:"Recall"
) else (
    echo Compilation failed
)
echo.

REM Restore original
copy MySolution.cpp.bak MySolution.cpp >nul
del MySolution.cpp.bak

echo ========================================
echo Comparison complete
echo ========================================
echo.
echo Recommendation: Choose config with:
echo   - Recall@10 ^>= 98%%
echo   - Build time ^< 25 minutes
echo   - Search time ^< 2 seconds
