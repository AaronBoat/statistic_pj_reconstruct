@echo off
REM GLOVE v1.3 测试 - 优化纯 HNSW (放弃K-Means)
REM 
REM v2.0 失败原因: K-Means 局部ID/全局ID映射错误,召回率崩溃至 0.7%
REM 
REM v1.3 优化策略:
REM   1. 降低构建复杂度: M=16, ef_c=150 (对比 v1.2 的 M=20, ef_c=200)
REM   2. 提高搜索质量: ef_s=600 (对比 v1.2 的 450)
REM   3. 构建复杂度: 1.2M × 16 × 150 = 2.88B 操作 (v1.2 的 60%)
REM 
REM 预期性能:
REM   - 构建时间: 20-25min (v1.2 为 35.6min)
REM   - 召回率: 98%+ (ef_s=600 补偿图质量)
REM   - 查询时间: ~3-4ms (ef_s 更高)
REM   - 距离计算: ~30000-35000 per query

echo ========================================
echo   GLOVE v1.3 测试 (优化纯 HNSW)
echo ========================================
echo.
echo v1.2 问题: 构建 35.6min (超时), 召回 94.8%%
echo v1.3 策略: 降低 M/ef_c, 提高 ef_s
echo.
echo 启动测试...
echo 预计时间: 20-25 分钟
echo.

.\test_solution_v13.exe ..\data_o\data_o\glove
