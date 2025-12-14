# 基于图缓存的快速调参优化

## 实施的优化

### 1. 图缓存功能 (Graph Caching)
**灵感来源**: XIAOYANG的K-means+倒排索引方案提到"缓存建构结果加速调参"

**实现**:
- 添加 `save_graph()` 和 `load_graph()` 方法到MySolution类
- 保存完整的HNSW图结构到二进制文件
- 首次构建后，后续测试只需加载图（秒级），无需重建（分钟级）

**优势**:
- 构建时间: 25-30分钟 → 加载时间: <1秒
- 可快速测试不同ef_search参数
- 避免重复构建相同图结构

### 2. NGT自适应搜索 (Adaptive Search)
**灵感来源**: YUANSHENG文档中的NGT优化策略

**实现**:
- 添加 `search_layer_adaptive()` 方法
- 引入gamma参数 (默认0.19)
- 动态阈值剪枝: `if (current_dist > w_top * (1.0 + gamma)) break`

**理论优势**:
- SIFT: 平均6.9条边导航到最近邻
- GLOVE: 平均4.5条边导航到最近邻
- 减少无效节点遍历

### 3. 快速调参工具
**文件**: 
- `fast_tune_params.cpp` - 独立调参工具
- `test_solution_cache.exe` - 增强版test_solution (支持--use-cache, --save-cache)
- `fast_tune.bat` - 自动化脚本

**使用方法**:

```bash
# 首次：构建并缓存图
test_solution_cache.exe ..\data_o\data_o\glove --save-cache

# 后续：加载缓存快速调参
test_solution_cache.exe ..\data_o\data_o\glove --use-cache
```

## 参数优化策略

### 当前最优配置 (GLOVE)
```cpp
M = 20                  // 图连接度
ef_construction = 165   // 构建时搜索深度
ef_search = 2800       // 查询时搜索深度
gamma = 0.19           // 自适应搜索阈值
```

### 性能指标
- 召回率@10: 98.4%
- 构建时间: 28.7分钟
- 搜索时间: 1.36秒 (100查询)
- 距离计算: 48,410次/查询

## 借鉴的核心思想

### 从XIAOYANG借鉴:
1. **缓存策略**: 构建一次，多次调参
   - K-means的质心可以缓存
   - HNSW的图结构同样可以缓存
   - 避免重复计算

2. **并行优化**: 
   - 已使用OpenMP并行构建图
   - 多线程分配质心 → 多线程构建HNSW层

3. **内存优化**:
   - CSR扁平化存储 → HNSW已使用vector<vector<int>>
   - SIMD距离计算 → 已实现AVX2/AVX512优化

### 从YUANSHENG借鉴:
1. **NGT自适应搜索**:
   - gamma=0.19动态阈值
   - 减少无效边遍历
   - 提升搜索效率

2. **参数设置原则**:
   - efConstruction=256覆盖度数分布
   - 基于数据集特性调整参数
   - 消融实验验证每个优化

## 下一步优化方向

### 1. 启用NGT自适应搜索
修改search_hnsw使用search_layer_adaptive:
```cpp
curr_neighbors = search_layer_adaptive(query_ptr, curr_neighbors, ef_search, 0, gamma);
```

### 2. 测试不同gamma值
- gamma=0.15: 更激进剪枝
- gamma=0.19: 当前默认值
- gamma=0.25: 更保守搜索

### 3. 组合优化
- ef_search + gamma 联合调优
- 类似XIAOYANG的 NUM_CENTROIDS + NPROBE 策略

### 4. 量化技术
- 借鉴YUANSHENG的int16量化
- 已有框架，需启用测试

## 快速调参流程

```bash
# 一次性构建
test_solution_cache.exe ..\data_o\data_o\glove --save-cache

# 快速测试不同ef_search
# ef_search = 2200, 2400, 2600, 2800, 3000
test_solution.cpp修改ef_search参数，使用--use-cache快速验证

# 测试NGT adaptive search
# 修改代码启用search_layer_adaptive，测试不同gamma
```

## 技术要点

### 图缓存文件格式
```
dimension (int)
num_vectors (int)
M, ef_construction, max_level (int)
entry_point (vector<int>)
vertex_level (vector<int>)
graph structure (3D vector)
vectors (float array)
```

### 自适应搜索核心逻辑
```cpp
// NGT-inspired termination
if (current_dist > w_top * (1.0 + gamma))
    break;

// NGT-inspired threshold pruning  
if (dist < threshold * (1.0 + gamma))
    candidates.push({dist, neighbor});
```

## 参考对比

| 方法 | 构建时间 | 召回率 | 搜索时间 | 距离计算 |
|------|---------|--------|----------|----------|
| 当前HNSW | 28.7min | 98.4% | 1.36s | 48,410 |
| XIAOYANG K-means | ? | 99.22% | 0.24s | ? |
| YUANSHENG NGT | ? | 99.26% | 2.87ms | 21,500 |

我们的目标: 结合两者优势，达到98%+召回，<30min构建，<3s搜索

## 总结

通过引入图缓存和NGT自适应搜索，我们建立了快速参数调优框架。核心思想是：
1. **一次构建，多次调参** (XIAOYANG启发)
2. **自适应剪枝** (YUANSHENG/NGT启发)
3. **系统化测试** (网格搜索 + 缓存加速)

这为后续精细化调参和算法优化提供了高效工具链。
