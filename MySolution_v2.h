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
    // HNSW parameters (per-partition)
    int M;               // number of connections per vertex
    int ef_construction; // size of dynamic candidate list during construction
    int ef_search;       // size of dynamic candidate list during search
    int max_level;       // maximum level
    float ml;            // level multiplier

    // K-Means parameters
    int num_clusters;         // Number of partitions (K)
    int num_probe_clusters;   // How many clusters to probe during search

    // Data storage
    int dimension;
    int num_vectors;
    vector<float> vectors;
    
    // K-Means clustering
    vector<vector<float>> centroids;           // [k][dim] - cluster centers
    vector<vector<int>> cluster_assignments;   // [k] - vector IDs in each cluster
    
    // Per-partition HNSW
    vector<int> entry_points;                  // Entry point per cluster
    vector<int> max_levels;                    // Max level per cluster
    vector<vector<vector<vector<int>>>> graphs; // [cluster][level][vertex][neighbors]
    vector<vector<int>> vertex_levels;         // [cluster][vertex] - level assignment

    // Helper structures
    mt19937 rng;

    // Statistics
    mutable long long distance_computations;

    // Distance calculation
    inline float distance(const float *a, const float *b, int dim) const;

    // K-Means methods
    void kmeans_init_centroids();
    void kmeans_assign_vectors();
    bool kmeans_update_centroids();
    void build_kmeans(int k, int max_iterations = 20);

    // HNSW methods (per-partition)
    int random_level();
    vector<int> search_layer(const float *query, const vector<int> &entry_points,
                             int ef, int level, int cluster_id) const;
    void select_neighbors_heuristic(vector<int> &neighbors, int M_level, int cluster_id);
    void connect_neighbors(int local_vertex, int level, const vector<int> &neighbors, int cluster_id);
    void build_hnsw_partition(int cluster_id);
    
    // Search methods
    vector<int> find_nearest_clusters(const float *query, int num_probes) const;
    void search_hnsw(const vector<float> &query, int *res);

public:
    Solution();
    ~Solution();

    // Set parameters (call before build)
    void set_parameters(int M_val, int ef_c, int ef_s);
    void set_clustering_parameters(int k, int num_probes);

    void build(int d, const vector<float> &base);
    void search(const vector<float> &query, int *res);

    // Statistics methods
    long long get_distance_computations() const { return distance_computations; }
    void reset_distance_computations() { distance_computations = 0; }
};

#endif // MY_SOLUTION_H
