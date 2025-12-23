@echo off
chcp 65001 >nul
echo.
echo ╔═══════════════════════════════════════════════════════════════╗
echo ║          第八批优化：阈值调优实验                           ║
echo ╚═══════════════════════════════════════════════════════════════╝
echo.
echo 【目标】找到最优剪枝阈值，平衡速度和召回率
echo   - 召回率@10 ≥ 97%% (必须达标)
echo   - 搜索时间 < 10ms (目标)
echo   - 构建时间 < 2000s (限制)
echo.
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo.

set THRESHOLD=%1
if "%THRESHOLD%"=="" set THRESHOLD=1.5

echo 【当前测试】阈值 = %THRESHOLD%x
echo.

echo [1/3] 修改代码中的阈值...
powershell -Command ^
  "(Get-Content MySolution.cpp) -replace 'partial_d ^> max_dist_in_W \* [0-9.]+f', 'partial_d ^> max_dist_in_W * %THRESHOLD%f' | Set-Content MySolution.cpp"
echo ✓ 阈值已更新为 %THRESHOLD%x
echo.

echo [2/3] 编译优化版本...
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp test_solution.cpp MySolution.cpp -o test_solution.exe 2>nul
if errorlevel 1 (
    echo ✗ 编译失败
    pause
    exit /b 1
)
echo ✓ 编译成功
echo.

echo [3/3] 测试 GLOVE 数据集...
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
set OMP_NUM_THREADS=8
test_solution.exe ..\data_o\data_o\glove > glove_threshold_%THRESHOLD%.txt 2>&1
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo.

echo 【测试结果】
findstr /C:"Build time" /C:"Average search" /C:"Recall" glove_threshold_%THRESHOLD%.txt
echo.

echo 结果已保存至: glove_threshold_%THRESHOLD%.txt
pause
