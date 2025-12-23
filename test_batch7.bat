@echo off
chcp 65001 >nul
echo.
echo ╔═══════════════════════════════════════════════════════════════╗
echo ║          第七批优化验证：Layer 0 极致性能测试               ║
echo ╚═══════════════════════════════════════════════════════════════╝
echo.
echo 【优化技术】
echo   1. 固定数组替代优先队列 (预期减少 20%% 开销)
echo   2. 插入排序保持有序 (小规模 ef 更高效)
echo   3. 流水线预取 (预期减少 35%% 内存延迟)
echo   4. 贪婪指针回溯 (预期减少 15%% 搜索路径)
echo   5. 提前终止剪枝 (预期减少 10%% 无效计算)
echo.
echo 【性能目标】搜索时间: 17.25ms → 5ms
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo.

echo [1/2] 编译优化版本...
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp test_solution.cpp MySolution.cpp -o test_solution.exe 2>nul
if errorlevel 1 (
    echo ✗ 编译失败
    pause
    exit /b 1
)
echo ✓ 编译成功
echo.

echo [2/2] 测试 GLOVE 数据集...
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
set OMP_NUM_THREADS=8
test_solution.exe ..\data_o\data_o\glove
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo.
echo 测试完成！请检查搜索时间是否达到 5ms 以下。
pause
