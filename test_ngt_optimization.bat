@echo off
chcp 65001 >nul
echo ==========================================
echo NGT自适应搜索优化测试
echo ==========================================
echo.

REM 编译
echo [编译] test_solution.cpp
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp test_solution.cpp MySolution.cpp -o test_ngt.exe 2>nul
if errorlevel 1 (
    echo 编译失败！
    pause
    exit /b 1
)
echo 编译成功！
echo.

echo ==========================================
echo 测试1: 标准HNSW (gamma=0, 禁用自适应)
echo ==========================================
echo.
echo 参数: M=20, ef_c=165, ef_s=2800, gamma=0
echo.

REM 创建临时测试程序
echo 运行测试...
test_ngt.exe ..\data_o\data_o\glove | findstr /C:"Build time" /C:"Recall" /C:"Total search" /C:"Average distance"

echo.
echo ==========================================
echo 完成！
echo ==========================================
pause
