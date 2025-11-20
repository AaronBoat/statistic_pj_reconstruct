高精度低时延向量检索算法
FUDAN UNIVERSITY
2025/11/05
数据结构课程 PJ

01 向量检索算法背景
向量（Vector）是 AI 模型理解、学习和表达数据的基本形式。
在 AI 应用系统中，通常会通过特征提取，将文本、图像、视频、用户行为等数据转换成 AI 模型易于理解的特征向量（Embedding），如词向量（word embedding）、图像特征向量（image embedding）等。
在实际业务场景中，对向量进行“相似性检索”是一种常用的操作，在大模型 RAG、推荐系统、图像搜索、智能问答、向量数据库、分子结构分析等场景中都有广泛的应用。
当前，向量检索已成为 AI 应用系统必备的基础设施之一。
02 向量检索算法定义
在实际业务场景中，如果将查询向量 q 和向量底库 B 中的向量进行逐一比对，向量检索的计算量会非常大，无法满足客户业务场景的时延要求。
因此，业界主流方案是在向量底库上构建一个高性能的向量索引，检索时基于向量索引快速查询到对应的向量，从而能够大幅降低向量检索的计算开销，满足客户业务的时延要求。
03 向量检索主流算法
向量检索算法当前有 5 条主流技术路线：
暴力搜索算法
量化算法
空间划分算法
图检索算法
图检索 + 量化融合算法
其中图检索算法较其他类型的算法具有压倒性的性能优势，是当前业界的 SOTA 算法，但图检索算法也存在内存开销大、索引构建耗时长等问题。
04 向量检索算法参考资料
向量检索算法是工业界和学术界研究的热点方向，在网上有大量的学习资源。以下论文和算法库可供参考：
Wang, Mengzhao, et al. "A comprehensive survey and experimental comparison of graph-based approximate nearest neighbor search." arXiv preprint arXiv:2101.12631 (2021).
https://arxiv.org/abs/2101.12631
Malkov Y A, Yashunin D A. "Efficient and robust approximate nearest neighbor search using hierarchical navigable small world graphs." IEEE Transactions on Pattern Analysis and Machine Intelligence, 2018, 42(4): 824–836.
https://ieeexplore.ieee.org/document/8594636/
05 题目流程
提交网站：http://10.176.56.208:5000
上传代码要求：
代码需实现 Solution 类以及类里的 build 和 search 函数，类和函数实现需遵循如下接口定义：
以下是该页面 OCR 识别后整理的 Markdown 格式内容：

---

# 05 题目流程

- **网站**：`http://10.176.56.208:5000`

- **上传代码**：上传算法代码，代码需实现 `Solution` 类以及类里的 `build` 和 `search` 函数，类和函数实现需要遵循接口定义：

  - **a) build 函数**：读入向量底库，完成向量索引构建；
  
  - **b) search 函数**：基于构建好的向量索引，返回向量检索结果。

---

## Solution 类

| 项目             | 内容                                                                 |
|------------------|----------------------------------------------------------------------|
| **类定义**       | `class Solution`                                                     |
| **成员函数定义** | `void build(int d, const vector<float>& base)`<br>输入底库向量 `base` 和向量维度 `d`，完成索引构建<br>`base` 中的第 `0~d-1` 个元素是第一个向量，第 `d~2d-1` 个元素是第二个向量，以此类推 |
|                  | `void search(const vector<float>& query, int* res)`<br>返回向量底库中距离查询向量 `query` 最近的 10 个向量 ID<br>其中 `res` 是用来存放结果的数组，长度为 10，内存空间已申请好 |

---

### 文件结构要求：

- `MySolution.cpp`
- `MySolution.h`
- `MySolution.tar`

（注：左侧图示显示文件图标及名称，实际提交应包含 `.cpp`、`.h` 及打包的 `.tar` 文件）

---

> 复旦大学 FUDAN UNIVERSITY  
> （页眉与页脚均有“博学而笃志 切问而近思”校训）

--- 

平台执行与指标记录：
平台将执行上传的算法代码，并记录以下关键指标：
检索时延：每个查询向量的检索平均时延；
检索精度：本赛题将 Top-10 召回率 作为检索精度指标；
build 时延：索引构建时延。
返回结果：
判题器将记录上述指标并返回给平台网页前端，可视化后呈现。

07 任务注意点
每天只允许提交一次。
将会检查代码，有投机取巧、卡 bug 等行为将可能作零分处理。
PJ 最终报告不鼓励过长，一般在 10 页左右；过多不会有额外加分，但也不会扣分。
若使用大模型生成无意义话语，视情况扣分。
感谢倾听！
FUDAN UNIVERSITY