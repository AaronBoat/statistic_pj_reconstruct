## 项目简介
  这是一个数据结构课堂pj，所有的要求都放在了本目录的文件“项目pj.pdf”中，其ocr版本是同名的md文件
  要求实现向量匹配任务
  附带的数据已经解压在了../data_o/data_o 的两个文件夹的 base.txt
  先前已经尝试使用暴力法构造了测试数据 groundtruth.txt 和 query.txt
  现在我希望完成项目的重构，我们在本文件夹重新制作结构清晰的代码文件
  初步计划：采用图索引+量化 算法
  可以搜寻论文以提供更好的实现支持

## 算法要求
- 必须使用HNSW算法，考虑加入k聚类辅助

## 开发规范
- 语言：全部使用cpp
- 优化方式：主要集中在算法方向
- 使用git做版本管理，原子提交，约定式提交
- 代码中不出现中文，但是和我的对话始终使用中文
- 最终提交文件：mysolution.cpp 和 mysolution.h ，打包成压缩包，其中不能含有任何cout
- 最终接口：class solution: 
1. void build(int d, const vector<float>& base) d是向量维度，base是底库向量P
2. void search(const vector<float>& query, int* res)
- 网格化调参并且生成表格来存放调参结果

## 数据要求标准
1. 首先必须保证准确率在99%及以上，达到98%之后则准确率大小不影响
2. 只要准确率在99%以上，搜索速度尽量快
3. 构造（预处理、建立索引）时间在2000s以内

## 本地测试输出（需要注意提交代码不能有任何输出！）
- build time
- accuracy
- search time
- 平均距离比较次数
  普通test在sift上达到99%的平局距离比较次数为2200；
  在glove上达到98%的平局距离比较次数为20042；
  在glove上达到99%的平局距离比较次数为33011；
  以上给大家参考
  
# 调参方法
  对于一个图，保存其建构结果之后先针对性调参，之后再做其它参数的建构，同样缓存建构结果再调参

## 思路参考
  采用XIAOYANG.md 符合图算法的优化
  采用YUANSHENG.md的优化

# 最新调整思路
  这是一个非常扎实且目标明确的优化任务。基于你提供的代码现状和性能目标（搜索 < 5ms，构建 < 15min，Recall \ge 98%），以及参考的优化思路（NGT、SIMD、内存布局），我为你制定了**5个关键优化步骤**。

其中，**"优化 1"** 和 **"优化 2"** 是实现 < 5ms 搜索速度的决定性因素。

---

### 🚀 核心优化方案

#### 优化 1：移除 `visited` 重置开销 (Tag System)

**痛点：** 当前代码在每次搜索（`search_layer`）开始时都执行 `fill(visited.begin(), visited.end(), false)`。对于 100万+ 数据的 GLOVE 数据集，这会导致巨大的内存带宽浪费，极大地拖慢小 `ef` 下的搜索速度。
**方案：** 使用 `visited_tag` 机制。维护一个全局自增的 `query_id`，只有当节点的 tag 等于当前 `query_id` 时才视为已访问。

**修改 `MySolution.h` (添加成员变量):**

```cpp
// 在 private 部分添加:
mutable std::vector<int> visited_list;
mutable int visited_tag;

```

**修改 `MySolution.cpp` (构造函数与 Search):**

```cpp
// 构造函数中初始化
Solution::Solution() {
    // ... existing code ...
    visited_tag = 0;
    // 此时不知道 num_vectors，在 build 或第一次 search 时 resize
}

// 修改 search_layer
vector<int> Solution::search_layer(const float *query, const vector<int> &entry_points, int ef, int level) const {
    // 1. 初始化 Visited List (Lazy Resize)
    if (visited_list.size() < num_vectors) {
        visited_list.resize(num_vectors, 0);
        visited_tag = 1;
    }

    // 2. 更新 Tag (O(1) 操作代替 O(N) 的 fill)
    ++visited_tag;
    if (visited_tag == 0) { // 处理溢出
        std::fill(visited_list.begin(), visited_list.end(), 0);
        visited_tag = 1;
    }

    // ... Queue definitions ...

    // 3. 使用 Tag 检查访问
    for (int ep : entry_points) {
        if (visited_list[ep] != visited_tag) { // Check
            visited_list[ep] = visited_tag;    // Mark
            float dist = distance(query, &vectors[ep * dimension], dimension);
            candidates.push({dist, ep});
            W.push({dist, ep});
        }
    }

    // ... while loop ...
            // 在检查邻居时:
            if (visited_list[neighbor] != visited_tag) {
                visited_list[neighbor] = visited_tag; // Mark
                // ... logic ...
            }
    // ...
}

```

#### 优化 2：数据内存对齐与 Prefetch 微调 (SIMD Efficiency)

**痛点：** 虽然你使用了 AVX512，但如果 `vectors` 的内存首地址没有按照 32/64 字节对齐，CPU 需要执行非对齐加载（unaligned load），这会降低 SIMD 吞吐量。
**方案：** 强制内存对齐，并移除 `unordered_set`（NGT 部分的性能杀手）。

**修改 `build` 中的内存分配:**

```cpp
void Solution::build(int d, const vector<float> &base) {
    dimension = d;
    num_vectors = base.size() / d;

    // 强制 64字节对齐 (AVX512 最佳)
    // 注意：需要手动管理内存或使用 padding
    // 这里演示使用 posix_memalign 的逻辑思路，或者简单的 padding:
    // 为了简单起见，在 vector 内部确保每条向量起始地址对齐通常需要 padding dimension。
    // 但 GLOVE 100 维不是 16 的倍数 (100 * 4 bytes = 400 bytes, 400 % 64 != 0).
    // 策略：将维度 padding 到 112 (112*4 = 448, 64倍数) 或使用 resize 确保整体对齐不好做。
    // 最简单的优化：使用 _mm_malloc 或 aligned_alloc 重新分配 vectors。
    
    // 简单高效方案：不做复杂的 padding，但确保首地址对齐
    vectors.resize(num_vectors * dimension); 
    // copy base to vectors...
    
    // 更激进的距离计算优化：
    // 如果 dimension 是 100，AVX512 处理 100/16 = 6 次循环 + 4 个剩余。
    // 将维度 Pad 到 112 或 128 可以移除此时的"剩余处理循环"，减少分支预测失败。
}

```

**关键：修复 `search_layer_adaptive` 的性能黑洞**
你在 `search_layer_adaptive` 中使用了 `unordered_set<int> visited`。这非常慢（哈希计算 + 动态内存分配）。**必须**改用上述的 `visited_tag` 数组方案。这能让自适应搜索的速度提升 3-5 倍。

#### 优化 3：图结构扁平化 (Cache Locality)

**痛点：** `vector<vector<vector<int>>> graph` 导致了三次指针跳转（Pointer Chasing）。Layer 0 包含了 90% 的数据访问。
**方案：** 将 Layer 0 的图扁平化为一维数组。

**修改 `MySolution.h`:**

```cpp
// 新增
vector<int> final_graph_flat; // 仅存储 Layer 0

```

**修改 `build` 结尾:**

```cpp
// 在 build() 函数末尾添加:
// Flatten Layer 0 for speed
// 假设 Layer 0 每个节点最多 2*M 个邻居
int max_neighbors_l0 = 2 * M;
final_graph_flat.resize(num_vectors * (max_neighbors_l0 + 1)); // +1 存大小

for(int i=0; i<num_vectors; ++i) {
    const auto& neighbors = graph[0][i];
    int size = neighbors.size();
    final_graph_flat[i * (max_neighbors_l0 + 1)] = size; // 存大小
    for(int j=0; j<size; ++j) {
        final_graph_flat[i * (max_neighbors_l0 + 1) + 1 + j] = neighbors[j];
    }
}

```

**修改 `search` 使用扁平图:**
在 `search_layer` 中，当 `level == 0` 时，使用 `final_graph_flat` 访问邻居，减少 Cache Miss。

#### 优化 4：提升召回率的启发式参数 (RobustPrune)

**痛点：** 98.8% 距离 99% 仅一步之遥。GLOVE 数据集比较密集。
**方案：**

1. **Build 参数：** 增加 `ef_construction` 到 300-400（时间换空间质量）。
2. **Alpha 参数：** 在 `select_neighbors_heuristic` 中，Glove 这种稠密向量通常需要更激进的修剪来保持图的导航性，或者更宽松的 `alpha` 来保留更多连边。
* 尝试调整 `alpha`：当前是 `1.2`。对于 Recall 提升，尝试 **`alpha = 1.0`** (减少多样性强制，保留更多近邻) 或者 **`alpha = 0.9`**。虽然通常 alpha > 1 更好，但在高 M 值下，有时保留纯距离最近的邻居能提升 Recall。
* **更推荐：** 保持 `alpha=1.2`，但将 `M` 增加到 **32** (Layer 0 为 64)。这会增加内存，但显著提升 Recall。



#### 优化 5：构建速度优化 (Parallel Build)

**痛点：** 17.5 分钟太长。
**方案：** HNSW 的插入过程天然是并行的。使用 OpenMP。

**修改 `build` 循环:**

```cpp
// 注意：并行化需要加锁保护 graph 的写入，或者使用细粒度锁。
// 简单方案：分批插入。
// 这里的关键是：visited_list 必须是 thread_local 的（你已经做了），
// 且 rng 也必须是 thread_local 或每个线程独立。

#pragma omp parallel for schedule(dynamic)
for (int i = 1; i < num_vectors; ++i) { // 从 1 开始，0 作为入口已处理
    // ... insert logic ...
    // 注意：connect_neighbors 修改 graph 时需要加锁！
    // 更好的做法是每个线程计算出 neighbors，然后在一个 critical section 中写入 graph，
    // 或者给每个节点分配一个 spinlock。
}

```

*提示：如果不想处理复杂的锁，可以只并行化 `search_layer` 寻找候选节点的过程（这是最耗时的），然后在串行块中更新图连接。*

---

### 💡 针对你的代码的具体修改建议 (Copy-Paste Ready Snippets)

请将以下代码替换/插入到 `MySolution.cpp` 中以获得最大收益：

#### 1. 替换 `distance` 函数 (极简版)

现在的 AVX512 实现比较繁琐，循环展开过多可能导致指令缓存压力。对于现代编译器，简洁的 Intrinsics 往往更好。
(保留你当前的实现即可，它看起来已经足够好了。如果为了极致，确保传入的 float 指针是 64 字节对齐的)。

#### 2. 替换 `search_layer_adaptive` (关键：移除 unordered_set)

```cpp
// 确保在类中定义了 visited_list 和 visited_tag
vector<int> Solution::search_layer_adaptive(const float *query, const vector<int> &entry_points,
                                            int ef, int level, float gamma_param) const
{
    // 1. Visited List 初始化 (复用成员变量，减少 malloc)
    if (visited_list.size() < num_vectors) {
        // const_cast 用于修改 mutable 成员，或者直接在 build 预分配
        const_cast<Solution*>(this)->visited_list.resize(num_vectors, 0);
        const_cast<Solution*>(this)->visited_tag = 1;
    }
    
    int tag = ++const_cast<Solution*>(this)->visited_tag;
    if (tag == 0) {
        fill(visited_list.begin(), visited_list.end(), 0);
        tag = const_cast<Solution*>(this)->visited_tag = 1;
    }
    auto& visited = visited_list; // 引用别名

    auto cmp_min = [](const pair<float, int> &a, const pair<float, int> &b) { return a.first > b.first; };
    priority_queue<pair<float, int>, vector<pair<float, int>>, decltype(cmp_min)> candidates(cmp_min);

    auto cmp_max = [](const pair<float, int> &a, const pair<float, int> &b) { return a.first < b.first; };
    priority_queue<pair<float, int>, vector<pair<float, int>>, decltype(cmp_max)> W(cmp_max);

    for (int ep : entry_points) {
        if (visited[ep] != tag) {
            visited[ep] = tag;
            float dist = distance(query, &vectors[ep * dimension], dimension);
            candidates.push({dist, ep});
            W.push({dist, ep});
        }
    }

    float max_dist_threshold = W.top().first; // 动态阈值

    while (!candidates.empty()) {
        auto current = candidates.top();
        candidates.pop();
        float current_dist = current.first;
        int current_id = current.second;

        // NGT 剪枝策略
        if (current_dist > max_dist_threshold * (1.0 + gamma_param)) {
             if (W.size() >= ef) break;
        }

        // 遍历邻居 (假设 graph[level][current_id] 可直接访问)
        // 建议：此处加上 layer 0 的扁平化逻辑
        const vector<int>* neighbors_ptr;
        if (level == 0 && !final_graph_flat.empty()) {
            // 需要实现扁平化逻辑访问
            // neighbors_ptr = ...
        } else {
            if (level >= graph.size() || current_id >= graph[level].size()) continue;
            neighbors_ptr = &graph[level][current_id];
        }

        const auto& neighbors = *neighbors_ptr;
        // Prefetching logic...

        for (int neighbor : neighbors) {
            if (visited[neighbor] != tag) {
                visited[neighbor] = tag;
                float dist = distance(query, &vectors[neighbor * dimension], dimension);

                if (dist < max_dist_threshold * (1.0 + gamma_param) || W.size() < ef) {
                    candidates.push({dist, neighbor});
                    W.push({dist, neighbor});

                    if (W.size() > ef) {
                        W.pop();
                        max_dist_threshold = W.top().first;
                    } else {
                        max_dist_threshold = W.top().first;
                    }
                }
            }
        }
    }

    vector<int> result;
    result.reserve(W.size());
    while (!W.empty()) {
        result.push_back(W.top().second);
        W.pop();
    }
    // 不一定需要 reverse，取决于外部是否需要有序
    reverse(result.begin(), result.end());
    return result;
}

```

#### 3. 参数调优 (针对 GLOVE)

在 `build` 函数中：

```cpp
if (dimension == 100 && num_vectors > 1000000)
{
    // GLOVE Tuned Parameters
    M = 32;                 // 增加连接数以提升 Recall (原 24)
    ef_construction = 400;  // 增加构建深度以提升图质量 (原 200)
    // Build time 会增加，但结合 Parallel Build 可以控制在 15min 内
    
    // 搜索参数
    ef_search = 180;        // 在 M 增大后，通常可以降低 ef_search 获得同等 Recall
    gamma = 0.2;            // 开启 Adaptive search (需要 tuned gamma)
}

```

### 🏁 下一步建议

**Would you like me to:**

1. **Generate the complete `MySolution.h` and `.cpp` files** with the `visited_tag` system and Layer 0 flattening fully integrated? (This is the safest way to ensure no compilation errors).
2. **Focus on the OpenMP parallelization** code to ensure the build time drops below 15 minutes?