# Project Structure

```
reconstruct/
│
├─── 核心提交文件 (提交到平台)
│    ├── MySolution.h          ★ 头文件
│    ├── MySolution.cpp        ★ 实现文件
│    └── MySolution.tar        ★ 提交包 (运行setup_all.bat后生成)
│
├─── 测试程序
│    ├── test_simple.cpp       简单测试（合成数据）
│    └── test_solution.cpp     完整测试（真实数据）
│
├─── 构建脚本 (Windows)
│    ├── setup_all.bat         ★ 一键完成所有步骤（推荐使用）
│    ├── build.bat             编译完整测试程序
│    ├── test_simple.bat       编译运行简单测试
│    ├── package.bat           创建提交包
│    └── init_git.bat          初始化Git仓库
│
├─── 构建配置 (Unix/Linux)
│    └── Makefile              Make构建配置
│
├─── Git版本控制
│    ├── .gitignore            Git忽略配置
│    └── commit_robustprune.bat 提交辅助脚本
│
├─── 文档 (中英文)
│    ├── README.md             ★ 详细项目文档（英文）
│    ├── QUICKSTART.md         快速参考指南（英文）
│    ├── SUMMARY.md            实现总结（中文）
│    ├── DEVLOG.md             开发日志（中文）
│    ├── STATUS.txt            项目状态（中文）
│    └── 使用说明.md           快速使用说明（中文）
│
└─── 需求文档 (已存在)
     ├── AGENT.md              开发需求
     ├── pj介绍.md             项目介绍
     └── 项目PJ.pdf            课程要求

```

## 文件说明

### ★ 必需文件

这些是提交到平台的核心文件：

1. **MySolution.h** - Solution类接口定义
   - 声明build()和search()函数
   - 定义HNSW数据结构
   - 包含必要的头文件

2. **MySolution.cpp** - HNSW算法实现
   - 完整的HNSW构建和搜索逻辑
   - RobustPrune邻居选择策略
   - 优化的距离计算

3. **MySolution.tar** - 提交包
   - 包含MySolution.h和MySolution.cpp
   - 由setup_all.bat或package.bat生成

### 测试程序

- **test_simple.cpp** - 100个4维向量的快速测试
- **test_solution.cpp** - 支持SIFT/GLOVE数据集的完整测试

### 一键脚本

- **setup_all.bat** ⭐ 推荐使用
  - 初始化Git → 编译 → 测试 → 打包
  - 一条命令完成所有工作

### 文档层次

1. **新手入门**：使用说明.md → QUICKSTART.md
2. **详细了解**：README.md → SUMMARY.md
3. **开发细节**：DEVLOG.md → STATUS.txt

## 使用流程

```
1. 打开命令提示符
   ↓
2. cd c:\codes\data_pj\reconstruct
   ↓
3. setup_all.bat
   ↓
4. 查看生成的 MySolution.tar
   ↓
5. 上传到平台
```

## 代码统计

- 核心代码：~300 行 (MySolution.cpp + MySolution.h)
- 测试代码：~250 行 (test_*.cpp)
- 文档：~500 行 (各种.md文件)
- 总计：~1000+ 行

## 算法复杂度

- 构建时间：O(N log N)
- 查询时间：O(log N)
- 空间开销：O(N * M)

其中 N = 向量数量，M = 每层连接数

## Git提交

预期提交历史：
```
* feat: add RobustPrune heuristic for neighbor selection
* feat: initial HNSW implementation for vector retrieval
```

## 关键特性

✓ HNSW图索引
✓ RobustPrune策略
✓ L2距离度量
✓ 多层图结构
✓ 快速查询
✓ 高召回率

## 性能目标

- 构建时延：可接受（一次性成本）
- 查询时延：< 1ms (目标)
- 召回率@10：> 90% (目标)

## 平台要求

接口：
```cpp
class Solution {
    void build(int d, const vector<float>& base);
    void search(const vector<float>& query, int* res);
};
```

提交：
- MySolution.h
- MySolution.cpp
- 打包为 MySolution.tar

## 检查点

- [ ] 11/26 - SIFT数据集 (20%)
- [ ] 12/10 - GLOVE数据集 (10%)
- [ ] 12/17-12/24 - 方法分享

## 注意事项

⚠️ 每天只能提交一次
⚠️ 代码无中文
⚠️ 提交前充分测试
