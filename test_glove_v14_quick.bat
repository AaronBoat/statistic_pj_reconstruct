@echo off
REM GLOVE v1.4 快速测试 (最终优化版本)
REM 
REM 版本历史:
REM   v1.2: M=20, ef_c=200, ef_s=450 → 35.6min, 94.8% recall
REM   v1.3: M=16, ef_c=150, ef_s=600 → ~25min, 92.9% recall
REM   v1.4: M=18, ef_c=180, ef_s=800 → 预期 25-28min, 98%+ recall
REM 
REM v1.4 优化:
REM   1. 平衡参数: M=18, ef_c=180 (v1.2 和 v1.3 之间)
REM   2. 高搜索质量: ef_s=800 (非常充分的搜索)
REM   3. OpenMP 并行: 加速构建过程
REM 
REM 预期性能:
REM   - 构建时间: 25-28min (OpenMP 可能进一步加速)
REM   - 召回率: 98%+ (ef_s=800 保证高质量)
REM   - 查询时间: ~4-5ms
REM   - 距离计算: ~40000 per query

echo ========================================
echo   GLOVE v1.4 快速测试 (最终版)
echo ========================================
echo.
echo 参数: M=18, ef_c=180, ef_s=800
echo OpenMP: 启用并行加速
echo.
echo 启动测试...
echo.

.\test_solution_v14.exe ..\data_o\data_o\glove
