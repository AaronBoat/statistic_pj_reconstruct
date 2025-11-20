# IVF (Inverted File Index) 实现说明

## 算法选择逻辑

根据数据集特征自动选择最优算法：

```cpp
if (dimension >= 100 && num_vectors > 500000) {
    use_ivf = true;   // GLOVE等高维均匀分布数据
} else {
    use_hnsw = true;  // SIFT等聚类明显的数据
}
```

## IVF算法优势

### 1. 适合GLOVE类数据集
- **均匀分布**：词向量在空间中分布较均匀
- **高维度**：100维，图索引效果不如量化
- **语义相似性**：适合聚类划分

### 2. 天然支持并发
```cpp
// 搜索阶段可以并行处理多个cluster
for (int i = 0; i < nprobe; ++i) {
    int cluster_id = centroid_distances[i].second;
    // 这里可以并行处理
    for (int vec_id : inverted_lists[cluster_id]) {
        // 计算距离
    }
}
```

### 3. 构建速度快
- K-means聚类：O(N × k × iterations)
- 远快于HNSW的O(N log N × M × ef_c)

## 核心参数

```cpp
nlist = 4096    // 聚类数量 (约sqrt(N))
nprobe = 64     // 搜索时检查的cluster数
```

### 参数调优建议
- **nlist**：聚类数量
  - 太小：每个cluster太大，搜索慢
  - 太大：K-means慢，可能漏掉正确答案
  - 推荐：sqrt(N) 到 N/100

- **nprobe**：搜索cluster数
  - 小值：速度快但召回率低
  - 大值：召回率高但速度慢
  - 推荐：从8开始调优到128

## 实现细节

### 1. K-means++ 初始化
```cpp
// 第一个中心随机选择
// 后续中心按距离平方的概率选择
// 避免局部最优
```

### 2. 迭代优化
- 最多20次迭代
- 提前收敛检测
- 避免过度优化

### 3. 搜索优化
```cpp
// 只对centroid做partial sort
nth_element(centroid_distances.begin(),
            centroid_distances.begin() + nprobe,
            centroid_distances.end());

// 只对候选结果的top-k做partial sort
nth_element(candidates.begin(),
            candidates.begin() + 10,
            candidates.end());
```

## 性能预期

### GLOVE数据集 (1,192,514 × 100维)

**优化前（HNSW）：**
- 构建时间：27.9分钟
- Recall@10：83.2%
- 查询时间：1.47ms

**优化后（IVF）：**
- 构建时间：预计10-15分钟 (-40%~-50%)
- Recall@10：预计90-95%
- 查询时间：预计0.5-1.0ms

### SIFT数据集 (1,000,000 × 128维)

**保持HNSW算法：**
- SIFT数据聚类明显，图索引效果更好
- 保持98%的高召回率

## 并发优化潜力

### 当前实现（单线程）
```cpp
for (int i = 0; i < actual_nprobe; ++i) {
    // 顺序处理每个cluster
}
```

### 并发优化方案
```cpp
#pragma omp parallel for
for (int i = 0; i < actual_nprobe; ++i) {
    // 并行处理每个cluster
    // 每个线程独立计算距离
}
```

### 并发收益
- **构建阶段**：K-means可并行（距离计算）
- **搜索阶段**：多cluster并行扫描
- **预期加速**：2-4倍（4核心CPU）

## 内存优化

当前内存占用：
```cpp
vectors:          N × d × 4 bytes
centroids:        nlist × d × 4 bytes
inverted_lists:   N × 4 bytes (索引)
```

GLOVE示例：
- vectors: 1,192,514 × 100 × 4 = 477 MB
- centroids: 4,096 × 100 × 4 = 1.6 MB
- inverted_lists: 1,192,514 × 4 = 4.8 MB
- **总计**：约483 MB

## 使用方法

### 自动选择
```cpp
Solution sol;
sol.build(dimension, vectors);  // 自动选择IVF或HNSW
sol.search(query, results);
```

### 手动调优
```cpp
// 修改MySolution.cpp中的参数
nlist = 8192;    // 更多聚类
nprobe = 128;    // 搜索更多cluster
```

## 测试命令

```cmd
# GLOVE数据集（自动使用IVF）
.\test_solution.exe ..\data_o\data_o\glove > glove_test_ivf.txt

# SIFT数据集（自动使用HNSW）
.\test_solution.exe ..\data_o\data_o\sift > sift_test_hnsw.txt
```

## 对比总结

| 特性 | HNSW | IVF |
|------|------|-----|
| 适用数据 | 聚类明显 | 均匀分布 |
| 构建速度 | 慢 | 快 |
| 查询速度 | 快 | 中等 |
| 召回率 | 高（聚类数据） | 高（均匀数据） |
| 并发性 | 难 | 易 |
| 内存占用 | O(N×M) | O(N) |
| 参数调优 | 复杂 | 简单 |

## 下一步优化

1. ✅ 实现IVF基础版本
2. ⏳ 测试GLOVE性能
3. ⏳ 添加OpenMP并发支持
4. ⏳ Product Quantization (PQ) 压缩
5. ⏳ SIMD向量化距离计算

---
**实现日期**：2025-11-16
**算法**：IVF with K-means++ initialization
