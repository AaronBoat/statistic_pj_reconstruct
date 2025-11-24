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
    // Algorithm selection
    bool use_ivf; // true for IVF, false for HNSW

    // HNSW parameters
    int M;               // number of connections per vertex
    int ef_construction; // size of dynamic candidate list during construction
    int ef_search;       // size of dynamic candidate list during search
    int max_level;       // maximum level
    float ml;            // level multiplier

    // IVF parameters
    int nlist;                          // number of clusters
    int nprobe;                         // number of clusters to search
    vector<vector<float>> centroids;    // cluster centroids
    vector<vector<int>> inverted_lists; // inverted lists: cluster_id -> list of vector ids

    // Data storage
    int dimension;
    int num_vectors;
    vector<float> vectors;
    vector<int> entry_point;

    // HNSW graph structure
    // graph[level][vertex_id] = list of neighbors
    vector<vector<vector<int>>> graph;
    vector<int> vertex_level;

    // Helper structures
    mt19937 rng;

    // Distance calculation
    float distance(const float *a, const float *b, int dim) const;

    // HNSW methods
    int random_level();
    vector<int> search_layer(const float *query, const vector<int> &entry_points,
                             int ef, int level) const;
    void select_neighbors_heuristic(vector<int> &neighbors, int M_level);
    void connect_neighbors(int vertex, int level, const vector<int> &neighbors);
    void build_hnsw();
    void search_hnsw(const vector<float> &query, int *res);

    // IVF methods
    void build_ivf();
    void kmeans_clustering();
    int assign_to_nearest_centroid(const float *vec) const;
    void search_ivf(const float *query, int *res) const;

public:
    Solution();
    ~Solution();

    // Set HNSW parameters (call before build)
    void set_parameters(int M_val, int ef_c, int ef_s);

    void build(int d, const vector<float> &base);
    void search(const vector<float> &query, int *res);
};

#endif // MY_SOLUTION_H
