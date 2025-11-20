# Vector Retrieval Algorithm Implementation

This directory contains a clean reimplementation of the vector retrieval algorithm for the course project.

## Algorithm

The implementation uses **HNSW (Hierarchical Navigable Small World)** graph-based indexing for efficient approximate nearest neighbor search.

### Key Features

- **HNSW Graph Index**: Hierarchical graph structure for fast navigation
- **RobustPrune Neighbor Selection**: Diversity-aware heuristic to avoid clustering
- **L2 Distance**: Euclidean distance metric
- **Configurable Parameters**: M (connections per node), ef_construction, ef_search

### Parameters

- `M = 16`: Number of bidirectional links per node
- `ef_construction = 200`: Size of dynamic candidate list during construction
- `ef_search = 100`: Size of dynamic candidate list during search
- `ml = 1/ln(2)`: Level assignment multiplier

### Algorithm Optimizations

1. **Multi-layer Graph**: Upper layers are sparse for fast navigation, layer 0 is dense for accuracy
2. **RobustPrune Strategy**: Selects diverse neighbors with 1.2x tolerance factor to prevent clustering
3. **Bidirectional Links**: Automatically maintains bidirectional connections with degree constraints
4. **Greedy Best-First Search**: Efficient search using priority queues

## Files

- `MySolution.h`: Header file with Solution class declaration
- `MySolution.cpp`: Implementation of Solution class with HNSW algorithm
- `test_solution.cpp`: Testing program with real data loading
- `test_simple.cpp`: Simple synthetic data test
- `README.md`: This file
- `DEVLOG.md`: Development log with implementation details
- `Makefile`: Build configuration for Unix-like systems
- `*.bat`: Build scripts for Windows

## Usage

### Quick Start (Windows)

Run the complete setup:
```cmd
setup_all.bat
```

This will:
1. Initialize git repository
2. Compile and run simple test
3. Create submission package (MySolution.tar)

### Individual Steps

#### Build Simple Test
```cmd
test_simple.bat
```

#### Build Full Test
```cmd
build.bat
```

#### Create Package
```cmd
package.bat
```

### Unix/Linux Build

```bash
make
./test_solution
make tar
```

## Interface

### Solution Class

```cpp
class Solution {
public:
    void build(int d, const vector<float>& base);
    void search(const vector<float>& query, int* res);
};
```

#### build()
Constructs HNSW index from base vectors.
- **Parameters**:
  - `d`: Dimension of vectors
  - `base`: Flat array of all vectors (size = num_vectors * d)
- **Complexity**: O(N * log(N) * M * ef_construction * d) where N is number of vectors

#### search()
Returns top 10 nearest neighbors for a query vector.
- **Parameters**:
  - `query`: Query vector (size = d)
  - `res`: Output array for result indices (size = 10, pre-allocated)
- **Complexity**: O(log(N) * M * ef_search * d)

## Performance Metrics

The platform evaluates:
- **Search Latency**: Average time per query
- **Recall@10**: Top-10 recall rate (accuracy metric)
- **Build Latency**: Time to construct the index

## Dataset Information

### SIFT (Week 12 Checkpoint - 20%)
- Feature vectors from image descriptors
- 128-dimensional vectors
- Focus: Completion check

### GLOVE (Week 14 Checkpoint - 10%)
- Word embedding vectors
- Higher dimensional vectors
- Focus: Performance optimization

## References

- Malkov, Y. A., & Yashunin, D. A. (2018). "Efficient and robust approximate nearest neighbor search using hierarchical navigable small world graphs." *IEEE Transactions on Pattern Analysis and Machine Intelligence*, 42(4), 824-836.
- Wang, M., et al. (2021). "A comprehensive survey and experimental comparison of graph-based approximate nearest neighbor search." *arXiv preprint arXiv:2101.12631*.

## Development

### Git Workflow

This project uses conventional commits:
- `feat:` - New features
- `fix:` - Bug fixes
- `perf:` - Performance improvements
- `docs:` - Documentation updates
- `refactor:` - Code refactoring

### Code Style

- No Chinese in code (comments and variable names in English)
- C++11 standard
- Optimize for algorithm efficiency
- Clear, readable code structure

## Next Steps

1. ✓ Initial HNSW implementation
2. ✓ RobustPrune neighbor selection
3. ⏳ Test with SIFT dataset
4. ⏳ Parameter tuning for better recall
5. ⏳ Consider Product Quantization for memory efficiency
6. ⏳ Test with GLOVE dataset
7. ⏳ Final report (≈10 pages)
