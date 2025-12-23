#ifndef MY_SOLUTION_H
#define MY_SOLUTION_H

#include <vector>
#include <queue>
#include <random>
#include <cmath>
#include <algorithm>
#include <limits>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <atomic>

using namespace std;

class Solution
{
private:
    // HNSW parameters
    int M;               // number of connections per vertex
    int ef_construction; // size of dynamic candidate list during construction
    int ef_search;       // size of dynamic candidate list during search
    int max_level;       // maximum level
    float ml;            // level multiplier
    float gamma;         // adaptive search threshold

    // Data storage
    int dimension;
    int num_vectors;
    vector<float> vectors;
    vector<int> entry_point; // vector to allow easy swap, though usually size 1

    // HNSW graph structure
    // graph[level][vertex_id] = list of neighbors
    vector<vector<vector<int>>> graph;
    vector<int> vertex_level;

    // Flattened Layer 0 for cache efficiency (Optimization 3)
    vector<int> final_graph_flat;

    // Fine-grained locks for parallel build
    struct NodeLock
    {
        std::atomic_flag lock = ATOMIC_FLAG_INIT;
        void acquire()
        {
            while (lock.test_and_set(std::memory_order_acquire))
            {
                // spin
            }
        }
        void release()
        {
            lock.clear(std::memory_order_release);
        }
    };
    // Note: NodeLock is not copyable/movable easily, so we manage it carefully or use deque/ptr
    // Using a pointer array or fixed vector to avoid reallocation issues
    vector<NodeLock> node_locks;

    // Helper structures
    mt19937 rng;
    mutable std::atomic<long long> distance_computations;

    // Distance calculation
    inline float distance(const float *a, const float *b, int dim) const;
    
    // 第八批优化：用于早期剪枝的部分距离计算
    inline float partial_distance(const float *a, const float *b, int dim) const;

    // HNSW methods
    int random_level();

    // Internal search that uses thread_local storage
    vector<int> search_layer(const float *query, const vector<int> &entry_points,
                             int ef, int level) const;

    vector<int> search_layer_adaptive(const float *query, const vector<int> &entry_points,
                                      int ef, int level, float gamma) const;

    void select_neighbors_heuristic(vector<int> &neighbors, int M_level);
    void connect_neighbors(int vertex, int level, const vector<int> &neighbors);

    void search_hnsw(const vector<float> &query, int *res);

public:
    Solution();
    ~Solution();

    void set_parameters(int M_val, int ef_c, int ef_s);
    void build(int d, const vector<float> &base);
    void search(const vector<float> &query, int *res);

    bool save_graph(const string &filename) const;
    bool load_graph(const string &filename);

    // Additional public interface for test harness
    void set_ef_search(int ef) { ef_search = ef; }
    void reset_distance_computations() { distance_computations.store(0); }
    long long get_distance_computations() const { return distance_computations.load(); }
};

#endif // MY_SOLUTION_H
