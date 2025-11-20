# 测试和调优指南

## 当前状态

代码已完成基础实现，现在需要测试和调优。

## 快速测试

### 方法1：运行简单测试
```cmd
cd c:\codes\data_pj\reconstruct
quick_test.bat
```

这将：
1. 编译test_simple.exe
2. 运行简单测试（100个4维向量）
3. 验证结果是否正确

### 方法2：完整设置
```cmd
setup_all.bat
```

## 最近的代码修改

### 修改1：简化select_neighbors_heuristic
- 暂时使用简单策略（选最近的M个邻居）
- 原因：RobustPrune需要额外的上下文信息
- 后续可以根据性能需求优化

### 修改2：修复connect_neighbors中的剪枝
- 在剪枝前按距离排序
- 确保保留最近的M个连接
- 避免随机剪枝导致的性能下降

### 修改3：增强test_simple.cpp
- 添加异常处理
- 更详细的输出
- 明确的成功/失败返回值

## 预期测试结果

简单测试应该：
1. 成功编译（无警告）
2. 构建索引（显示进度）
3. 返回正确结果（第一个结果是0）
4. 显示"Test PASSED"

## 如果测试失败

### 编译错误
- 检查g++版本：`g++ --version`
- 确保支持C++11
- 查看compile_errors.txt

### 运行时错误
- 检查内存访问越界
- 查看输出中的错误信息
- 使用更小的数据集测试

### 结果不正确
- 检查distance函数
- 验证search_layer逻辑
- 确认entry_point设置正确

## 下一步调优

1. **先跑通简单测试** ← 当前目标
2. 测试真实数据（SIFT小样本）
3. 调整参数（M, ef_construction, ef_search）
4. 测量性能指标
5. 优化瓶颈

## 参数调优建议

当前参数：
- M = 16
- ef_construction = 200
- ef_search = 100

调优方向：
- 提高召回率：增加ef_search
- 降低延迟：减少ef_search
- 提高精度：增加M和ef_construction

## 性能监控

关注指标：
- 构建时间
- 查询时间
- 召回率@10
- 内存占用

## 调试技巧

1. 在build()中添加更多进度输出
2. 在search()中输出中间结果
3. 检查graph结构的大小和连接数
4. 验证各层的节点分布

## 运行quick_test.bat即可开始！
