@echo off
REM GLOVE v2.0 测试 - HNSW + K-Means 混合索引
REM 
REM 优化策略:
REM   1. K-Means 预分区: 150 个簇,降低每个分区的构建复杂度
REM   2. 降低 HNSW 参数: M=12, ef_c=80 (每分区平均 ~8000 向量)
REM   3. 多分区探测: 查询时探测 4 个最近簇,合并结果
REM 
REM 预期性能:
REM   - 构建时间: K-Means (5-7min) + HNSW (15-20min) = 20-27min ✓
REM   - 召回率: 98%+ (多分区探测 + 充分搜索)
REM   - 查询时间: ~3-4ms (4个分区探测)
REM   - 距离计算: ~15000-20000 per query

echo ========================================
echo   GLOVE v2.0 测试 (HNSW + K-Means)
echo ========================================
echo.
echo 启动测试...
echo 预计时间: 20-27 分钟
echo.

.\test_solution_v2.exe ..\data_o\data_o\glove
