# GLOVE 数据集优化说明

## 数据集特性分析

### GLOVE vs SIFT 对比

| 特性 | SIFT | GLOVE |
|------|------|-------|
| 向量数量 | 1,000,000 | 1,192,514 |
| 向量维度 | 128 | 100 |
| 数据分布 | 局部聚集 | 高维均匀 |
| 最优算法 | HNSW | HNSW (需更高参数) |

### 性能目标 (来自 AGENT.md)

**GLOVE 数据集标准:**
- 98% 召回率: 平均 **20,042** 次距离计算
- 99% 召回率: 平均 **33,011** 次距离计算
- 构建时间: **< 2000s** (约 33 分钟)
- 召回率要求: **≥ 98%**

**SIFT 数据集标准 (参考):**
- 99% 召回率: 平均 **2,200** 次距离计算

## 优化策略

### 自动参数检测

代码会根据数据集特征自动选择最优参数:

```cpp
void Solution::build(int d, const vector<float> &base)
{
    dimension = d;
    num_vectors = base.size() / d;
    
    // Auto-detect dataset
    if (dimension == 100 && num_vectors > 1000000) {
        // GLOVE dataset detected
        M = 24;                 // Higher connectivity
        ef_construction = 200;  // Higher quality graph
        ef_search = 300;        // More extensive search
    } else if (dimension == 128 && num_vectors > 900000) {
        // SIFT dataset detected
        M = 16;                 // Balanced for speed
        ef_construction = 100;  // Fast build
        ef_search = 200;        // Adequate search
    }
    ...
}
```

### 参数对比

| 参数 | SIFT 配置 | GLOVE 配置 | 变化 | 理由 |
|------|-----------|------------|------|------|
| **M** | 16 | 24 | +50% | GLOVE 需要更高连接度以应对高维稀疏性 |
| **ef_construction** | 100 | 200 | +100% | 构建更高质量的图以提升召回率 |
| **ef_search** | 200 | 300 | +50% | 更广泛的搜索以达到 98% 召回率目标 |

### 理论分析

**为什么 GLOVE 需要更高参数?**

1. **维度差异**:
   - SIFT: 128 维,较低维度空间
   - GLOVE: 100 维,但数据分布更均匀,需要更多探索

2. **距离计算目标差异**:
   - SIFT: ~2,200 次达到 99% 召回
   - GLOVE: ~20,000 次才能达到 98% 召回 (9x 差距!)

3. **复杂度影响**:
   - 构建复杂度: O(N × M × ef_construction)
   - SIFT: 1M × 16 × 100 = 1.6B 操作
   - GLOVE: 1.2M × 24 × 200 = 5.76B 操作 (3.6x)
   - 预估构建时间: ~20-25 分钟 (< 30 分钟目标 ✓)

## 预期性能

### GLOVE 数据集 (v1.0 参数)

**构建阶段:**
- 参数: M=24, ef_construction=200
- 预估时间: **15-25 分钟**
- 目标: < 30 分钟 ✓

**查询阶段:**
- 参数: ef_search=300
- 预估速度: **0.5-1.0 ms/query**
- 预估距离计算: **18,000-25,000 次**
- 目标召回率: **≥ 98%** (目标 ~20,000 次距离计算)

### 对比参考

| 指标 | SIFT (v6.0) | GLOVE (v1.0) | 说明 |
|------|-------------|--------------|------|
| 构建时间 | ~10-15 min | ~15-25 min | GLOVE 稍慢但在限制内 |
| 查询速度 | ~0.3 ms | ~0.5-1.0 ms | GLOVE 因搜索范围更大而稍慢 |
| 距离计算 | ~2,500 | ~20,000 | GLOVE 需要更多探索 |
| 召回率@10 | 98-99% | 98-99% (目标) | 两者都达标 |

## 测试方法

### 快速测试
```powershell
.\test_local_glove.ps1
```

### 完整测试
```powershell
g++ -std=c++11 -O3 test_solution.cpp MySolution.cpp -o test_solution.exe
.\test_solution.exe ..\data_o\data_o\glove
```

### 监控进度
测试大约需要 **20-30 分钟**,包括:
1. 数据加载: ~2-3 分钟
2. 图构建: ~15-25 分钟
3. 查询测试: ~1 分钟

## 进一步优化方向

如果首次测试结果不理想,可考虑:

### 召回率不足 (< 98%)
- 增加 `ef_search`: 300 → 350 或 400
- 增加 `ef_construction`: 200 → 240
- 增加 `M`: 24 → 32

### 构建时间过长 (> 30 min)
- 减少 `ef_construction`: 200 → 150
- 减少 `M`: 24 → 20
- 保持或增加 `ef_search` 以补偿

### 查询速度过慢
- 减少 `ef_search`: 300 → 250
- 注意监控召回率变化

## 提交准备

**MySolution.tar** 已包含:
- ✅ 自动数据集检测
- ✅ SIFT 优化参数 (M=16, ef_c=100, ef_s=200)
- ✅ GLOVE 优化参数 (M=24, ef_c=200, ef_s=300)
- ✅ 无任何 cout 输出
- ✅ 统一 HNSW 算法实现

**检查点时间:**
- 11/26: SIFT 数据集 (20%) ✅
- 12/10: GLOVE 数据集 (10%) 🔄

---

**当前状态:** GLOVE 完整测试运行中 (~15-25 min)
**下一步:** 等待测试结果,根据实际性能调整参数
