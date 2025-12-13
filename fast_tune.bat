@echo off
chcp 65001 >nul
echo ==========================================
echo 快速参数调优 (使用图缓存)
echo ==========================================
echo.

REM 编译快速调参工具
echo [编译] fast_tune_params.cpp
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp fast_tune_params.cpp MySolution.cpp -o fast_tune_params.exe
if errorlevel 1 (
    echo 编译失败！
    pause
    exit /b 1
)
echo 编译成功！
echo.

REM 第一次运行：构建图并缓存
echo ==========================================
echo 步骤1: 构建图并缓存 (GLOVE数据集)
echo ==========================================
echo.
fast_tune_params.exe ..\data_o\data_o\glove 1
echo.

REM 第二次运行：使用缓存快速调参
echo ==========================================
echo 步骤2: 使用缓存快速测试多个参数
echo ==========================================
echo.
fast_tune_params.exe ..\data_o\data_o\glove 0
echo.

echo ==========================================
echo 调参完成！
echo ==========================================
pause
