@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ==========================================
echo 对比测试: 标准HNSW vs NGT自适应搜索
echo ==========================================
echo.

REM 备份当前MySolution.cpp
copy MySolution.cpp MySolution_backup_temp.cpp >nul

echo [步骤1] 测试标准HNSW (gamma=0)
echo ----------------------------------------
echo 修改gamma=0...
powershell -Command "(Get-Content MySolution.cpp) -replace 'gamma = [0-9.]+;', 'gamma = 0.0;' | Set-Content MySolution.cpp"

echo 编译...
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp test_solution.cpp MySolution.cpp -o test_compare.exe 2>nul
if errorlevel 1 (
    echo 编译失败！
    goto :cleanup
)

echo 运行测试...
echo.
test_compare.exe ..\data_o\data_o\glove | findstr /C:"Build time" /C:"Recall@10" /C:"Total search" /C:"Average distance" > result_gamma0.txt
type result_gamma0.txt
echo.

echo [步骤2] 测试NGT自适应 (gamma=0.19)
echo ----------------------------------------
echo 修改gamma=0.19...
powershell -Command "(Get-Content MySolution.cpp) -replace 'gamma = [0-9.]+;', 'gamma = 0.19;' | Set-Content MySolution.cpp"

echo 编译...
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp test_solution.cpp MySolution.cpp -o test_compare.exe 2>nul
if errorlevel 1 (
    echo 编译失败！
    goto :cleanup
)

echo 运行测试...
echo.
test_compare.exe ..\data_o\data_o\glove | findstr /C:"Build time" /C:"Recall@10" /C:"Total search" /C:"Average distance" > result_gamma019.txt
type result_gamma019.txt
echo.

echo ==========================================
echo 对比结果
echo ==========================================
echo.
echo 【标准HNSW (gamma=0)】
type result_gamma0.txt
echo.
echo 【NGT自适应 (gamma=0.19)】
type result_gamma019.txt
echo.

:cleanup
echo 恢复备份...
copy MySolution_backup_temp.cpp MySolution.cpp >nul
del MySolution_backup_temp.cpp >nul 2>&1
del test_compare.exe >nul 2>&1

echo.
echo ==========================================
echo 完成！
echo ==========================================
pause
