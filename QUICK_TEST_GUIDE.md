# 快速测试指南

## 🎯 提交包状态
- **文件**: `MySolution.tar` (13,312 bytes)
- **内容**: MySolution.h + MySolution.cpp
- **算法**: 纯 HNSW (图检索)
- **输出**: ✓ 无任何控制台输出

## 📋 本地测试命令

### 方法1: 快速验证 (推荐新手)
```bash
.\test_local_sift.bat
```
- ✓ 实时显示输出
- ✓ 简单直接
- ✗ 不保存日志

### 方法2: 完整测试 (推荐)
```powershell
.\test_local_sift.ps1
```
- ✓ 实时显示输出
- ✓ 自动保存日志到 `sift_test_result.txt`
- ✓ 自动提取关键指标
- ✓ 性能评估报告
- ✓ 彩色输出更清晰

### 方法3: 手动测试
```bash
# 编译
g++ -std=c++11 -O3 test_solution.cpp MySolution.cpp -o test_solution.exe

# 运行
.\test_solution.exe ..\data_o\data_o\sift
```

## 📊 性能指标要求

| 指标 | 目标值 | 预期值 |
|------|--------|--------|
| Recall@10 | ≥ 98% | ~99.4% |
| Build Time | < 30 min | ~18-22 min |
| Query Time | < 5 ms | ~1.94 ms |

## ⚙️ 当前参数

```cpp
M = 16                // 每个节点的连接数
ef_construction = 200 // 构建时搜索队列大小
ef_search = 400       // 查询时搜索队列大小
```

## 📦 提交流程

1. **本地测试** (推荐)
   ```bash
   .\test_local_sift.ps1
   ```

2. **检查指标**
   - Recall@10 ≥ 98%
   - Build time < 30 min
   - Query time < 5 ms

3. **提交文件**
   - 访问: http://10.176.56.208:5000
   - 上传: `MySolution.tar`

4. **注意事项**
   - ⚠️ 每天只能提交一次
   - ⚠️ 检查投机取巧行为
   - ⚠️ 确保无额外输出

## 🔍 测试数据集

### SIFT 数据集位置
```
..\data_o\data_o\sift\
  ├── sift_base.fvecs      (1,000,000 向量, 128维)
  ├── sift_query.fvecs     (10,000 查询向量)
  └── sift_groundtruth.ivecs (真实标签)
```

### 测试时长
- 预计时间: **20-25 分钟**
- 构建阶段: ~18-22 分钟
- 查询阶段: ~20 秒

## ✅ 提交前检查清单

- [ ] 编译无警告: `g++ -std=c++11 -O3 ...`
- [ ] 简单测试通过: `.\test_simple.exe`
- [ ] SIFT测试通过: `.\test_local_sift.ps1`
- [ ] Recall@10 ≥ 98%
- [ ] Build time < 30 min
- [ ] 代码无 cout/cerr/printf
- [ ] tar 包含 .h 和 .cpp 文件
- [ ] 确认每天只提交一次

## 🚀 准备就绪！

所有检查通过后，即可访问提交网站：
**http://10.176.56.208:5000**

---

*最后更新: 2024-11-25*
