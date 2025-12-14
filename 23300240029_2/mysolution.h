#ifndef MY_SOLUTION_H
#define MY_SOLUTION_H

#include <vector>
#include <queue>
#include <unordered_set>
#include <random>
#include <cmath>
#include <algorithm>
#include <limits>
#include <cstring>
#include <iostream>

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

    // Data storage
    int dimension;
    int num_vectors;
    vector<float> vectors;
    vector<int> entry_point;

    // Quantization parameters (optional)
    bool use_quantization;
    vector<unsigned char> quantized_vectors; // 8-bit quantized vectors
    vector<float> quantization_mins;         // Min values per dimension
    vector<float> quantization_scales;       // Scale factors per dimension

    // HNSW graph structure
    // graph[level][vertex_id] = list of neighbors
    vector<vector<vector<int>>> graph;
    vector<int> vertex_level;

    // Helper structures
    mt19937 rng;

    // Statistics
    mutable long long distance_computations;

    // Distance calculation
    inline float distance(const float *a, const float *b, int dim) const;
    inline float distance_quantized(int vec_id_a, const float *b) const;

    // Quantization methods
    void build_quantization();
    void quantize_vector(const float *vec, unsigned char *quantized) const;

    // HNSW methods
    int random_level();
    vector<int> search_layer(const float *query, const vector<int> &entry_points,
                             int ef, int level) const;
    void select_neighbors_heuristic(vector<int> &neighbors, int M_level);
    void connect_neighbors(int vertex, int level, const vector<int> &neighbors);
    void build_hnsw();
    void search_hnsw(const vector<float> &query, int *res);

public:
    Solution();
    ~Solution();

    // Set HNSW parameters (call before build)
    void set_parameters(int M_val, int ef_c, int ef_s);

    void build(int d, const vector<float> &base);
    void search(const vector<float> &query, int *res);

    // Statistics methods
    long long get_distance_computations() const { return distance_computations; }
    void reset_distance_computations() { distance_computations = 0; }
};

#endif // MY_SOLUTION_H
