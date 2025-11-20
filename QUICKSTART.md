# Quick Reference Guide

## 快速开始（推荐）

### 一键完成所有步骤
```cmd
setup_all.bat
```
这将自动：
1. 初始化Git仓库
2. 编译并测试代码
3. 生成提交包 MySolution.tar

## 分步操作

### 1. 初始化Git仓库
```cmd
init_git.bat
```

### 2. 测试代码（简单测试）
```cmd
test_simple.bat
```

### 3. 测试代码（完整测试）
```cmd
build.bat
```

### 4. 创建提交包
```cmd
package.bat
```

## 提交到平台

1. 找到生成的文件：`MySolution.tar`
2. 访问：http://10.176.56.208:5000
3. 上传 `MySolution.tar`

## 主要文件说明

| 文件 | 用途 |
|------|------|
| MySolution.h | 算法头文件（提交） |
| MySolution.cpp | 算法实现（提交） |
| MySolution.tar | 提交包（上传） |
| test_solution.cpp | 完整测试程序 |
| test_simple.cpp | 简单测试 |
| README.md | 详细文档 |
| SUMMARY.md | 实现总结 |
| DEVLOG.md | 开发日志 |

## 算法参数

当前配置（可在MySolution.cpp构造函数中修改）：
```cpp
M = 16;                  // 每层连接数
ef_construction = 200;   // 构建时搜索深度
ef_search = 100;         // 查询时搜索深度
```

## 性能调优提示

### 提高召回率（精度）
- 增加 `ef_search` (100 → 150 → 200)
- 增加 `M` (16 → 24 → 32)

### 降低查询时延
- 减少 `ef_search` (100 → 80 → 60)
- 保持 `M` 较小

### 平衡方案
- M = 16, ef_construction = 200, ef_search = 100 (当前)

## 常见问题

### Q: 编译失败？
A: 确保安装了 g++ 编译器：
```cmd
g++ --version
```

### Q: 找不到数据文件？
A: 检查路径 `../data_o/data_o/sift/base.txt` 是否存在

### Q: 如何修改参数？
A: 编辑 `MySolution.cpp`，在构造函数中修改参数值

### Q: 如何查看Git历史？
A: 
```cmd
git log --oneline
```

## 数据集位置

- SIFT: `../data_o/data_o/sift/`
  - base.txt: 底库向量
  - query.txt: 查询向量
  - groundtruth.txt: 真实结果

- GLOVE: `../data_o/data_o/glove/`
  - 同上

## 重新开始

如需重新编译：
```cmd
del *.exe *.o MySolution.tar
setup_all.bat
```

## 联系方式

- 项目位置: `c:\codes\data_pj\reconstruct\`
- 数据位置: `c:\codes\data_pj\data_o\data_o\`

## 检查清单

提交前检查：
- [ ] MySolution.h 存在
- [ ] MySolution.cpp 存在
- [ ] MySolution.tar 已生成
- [ ] 简单测试通过
- [ ] 代码无中文
- [ ] Git已提交

## 时间节点提醒

- 11/26: SIFT数据集检查（20%）
- 12/10: GLOVE数据集检查（10%）
- 12/17-12/24: 方法分享

每天只能提交一次！请谨慎测试后再上传。
