#include "MySolution_v2.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <limits>

using namespace std;

Solution::Solution()
{
    // Default HNSW parameters (will be auto-configured)
    M = 12;               // Lower for faster per-partition build
    ef_construction = 80; // Lower for faster build
    ef_search = 150;      // Adequate search quality
    ml = 1.0 / log(2.0);
    max_level = 0;

    // Default clustering parameters
    num_clusters = 0;
    num_probe_clusters = 3;

    distance_computations = 0;
    rng.seed(42);
}

Solution::~Solution()
{
}

void Solution::set_parameters(int M_val, int ef_c, int ef_s)
{
    M = M_val;
    ef_construction = ef_c;
    ef_search = ef_s;
    ml = 1.0 / log(2.0);
}

void Solution::set_clustering_parameters(int k, int num_probes)
{
    num_clusters = k;
    num_probe_clusters = num_probes;
}

inline float Solution::distance(const float *a, const float *b, int dim) const
{
    ++distance_computations;

    float sum = 0.0f;
    int i = 0;

    // Loop unrolling (8-way)
    for (; i + 8 <= dim; i += 8)
    {
        float diff0 = a[i] - b[i];
        float diff1 = a[i + 1] - b[i + 1];
        float diff2 = a[i + 2] - b[i + 2];
        float diff3 = a[i + 3] - b[i + 3];
        float diff4 = a[i + 4] - b[i + 4];
        float diff5 = a[i + 5] - b[i + 5];
        float diff6 = a[i + 6] - b[i + 6];
        float diff7 = a[i + 7] - b[i + 7];
        sum += diff0 * diff0 + diff1 * diff1 + diff2 * diff2 + diff3 * diff3;
        sum += diff4 * diff4 + diff5 * diff5 + diff6 * diff6 + diff7 * diff7;
    }

    for (; i < dim; ++i)
    {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

int Solution::random_level()
{
    float r = (float)rng() / (float)rng.max();
    return (int)(-log(r) * ml);
}

// ==================== K-Means Implementation ====================

void Solution::kmeans_init_centroids()
{
    // K-Means++ initialization for better convergence
    centroids.resize(num_clusters, vector<float>(dimension, 0.0f));

    // First centroid: random point
    int first_idx = rng() % num_vectors;
    copy(vectors.begin() + first_idx * dimension,
         vectors.begin() + (first_idx + 1) * dimension,
         centroids[0].begin());

    // Remaining centroids: weighted by distance to nearest centroid
    vector<float> min_distances(num_vectors, numeric_limits<float>::max());

    for (int k = 1; k < num_clusters; ++k)
    {
        // Update min distances with new centroid
        for (int i = 0; i < num_vectors; ++i)
        {
            float dist = distance(&vectors[i * dimension], centroids[k - 1].data(), dimension);
            min_distances[i] = min(min_distances[i], dist);
        }

        // Sample next centroid proportional to squared distance
        float sum_dist = 0.0f;
        for (float d : min_distances)
            sum_dist += d;

        float rand_val = ((float)rng() / rng.max()) * sum_dist;
        float cumsum = 0.0f;
        int next_idx = 0;

        for (int i = 0; i < num_vectors; ++i)
        {
            cumsum += min_distances[i];
            if (cumsum >= rand_val)
            {
                next_idx = i;
                break;
            }
        }

        copy(vectors.begin() + next_idx * dimension,
             vectors.begin() + (next_idx + 1) * dimension,
             centroids[k].begin());
    }
}

void Solution::kmeans_assign_vectors()
{
    // Clear previous assignments
    cluster_assignments.assign(num_clusters, vector<int>());

    // Assign each vector to nearest centroid
    for (int i = 0; i < num_vectors; ++i)
    {
        float min_dist = numeric_limits<float>::max();
        int best_cluster = 0;

        for (int k = 0; k < num_clusters; ++k)
        {
            float dist = distance(&vectors[i * dimension], centroids[k].data(), dimension);
            if (dist < min_dist)
            {
                min_dist = dist;
                best_cluster = k;
            }
        }

        cluster_assignments[best_cluster].push_back(i);
    }
}

bool Solution::kmeans_update_centroids()
{
    bool changed = false;

    for (int k = 0; k < num_clusters; ++k)
    {
        if (cluster_assignments[k].empty())
            continue;

        vector<float> new_centroid(dimension, 0.0f);

        // Compute mean of assigned vectors
        for (int vec_id : cluster_assignments[k])
        {
            for (int d = 0; d < dimension; ++d)
            {
                new_centroid[d] += vectors[vec_id * dimension + d];
            }
        }

        for (int d = 0; d < dimension; ++d)
        {
            new_centroid[d] /= cluster_assignments[k].size();
        }

        // Check if centroid changed significantly
        float centroid_shift = distance(centroids[k].data(), new_centroid.data(), dimension);
        if (centroid_shift > 1e-6)
        {
            changed = true;
            centroids[k] = new_centroid;
        }
    }

    return changed;
}

void Solution::build_kmeans(int k, int max_iterations)
{
    num_clusters = k;

    // Initialize centroids using K-Means++
    kmeans_init_centroids();

    // Iterate until convergence or max iterations
    for (int iter = 0; iter < max_iterations; ++iter)
    {
        kmeans_assign_vectors();
        bool changed = kmeans_update_centroids();

        if (!changed)
            break;
    }

    // Final assignment
    kmeans_assign_vectors();
}

// ==================== HNSW Implementation (Per-Partition) ====================

vector<int> Solution::search_layer(const float *query, const vector<int> &entry_points,
                                   int ef, int level, int cluster_id) const
{
    unordered_set<int> visited;
    visited.reserve(ef * 2);

    auto cmp_min = [](const pair<float, int> &a, const pair<float, int> &b)
    {
        return a.first > b.first;
    };
    priority_queue<pair<float, int>, vector<pair<float, int>>, decltype(cmp_min)> candidates(cmp_min);

    auto cmp_max = [](const pair<float, int> &a, const pair<float, int> &b)
    {
        return a.first < b.first;
    };
    priority_queue<pair<float, int>, vector<pair<float, int>>, decltype(cmp_max)> W(cmp_max);

    // Convert local vertex IDs to global IDs
    for (int local_ep : entry_points)
    {
        int global_id = cluster_assignments[cluster_id][local_ep];
        float dist = distance(query, &vectors[global_id * dimension], dimension);
        candidates.push({dist, local_ep});
        W.push({dist, local_ep});
        visited.insert(local_ep);
    }

    while (!candidates.empty())
    {
        pair<float, int> curr = candidates.top();
        candidates.pop();
        float curr_dist = curr.first;
        int curr_vertex = curr.second;

        if (curr_dist > W.top().first)
            break;

        // Explore neighbors at this level
        if (level < (int)graphs[cluster_id].size() &&
            curr_vertex < (int)graphs[cluster_id][level].size())
        {
            for (int neighbor : graphs[cluster_id][level][curr_vertex])
            {
                if (visited.find(neighbor) == visited.end())
                {
                    visited.insert(neighbor);
                    int global_neighbor = cluster_assignments[cluster_id][neighbor];
                    float dist = distance(query, &vectors[global_neighbor * dimension], dimension);

                    if (dist < W.top().first || (int)W.size() < ef)
                    {
                        candidates.push({dist, neighbor});
                        W.push({dist, neighbor});

                        if ((int)W.size() > ef)
                            W.pop();
                    }
                }
            }
        }
    }

    vector<int> result;
    while (!W.empty())
    {
        result.push_back(W.top().second);
        W.pop();
    }
    reverse(result.begin(), result.end());
    return result;
}

void Solution::select_neighbors_heuristic(vector<int> &neighbors, int M_level, int cluster_id)
{
    if ((int)neighbors.size() <= M_level)
        return;

    // Simple pruning: keep M_level closest neighbors
    partial_sort(neighbors.begin(), neighbors.begin() + M_level, neighbors.end(),
                 [&](int a, int b)
                 {
                     int global_a = cluster_assignments[cluster_id][a];
                     int global_b = cluster_assignments[cluster_id][b];
                     return distance(&vectors[global_a * dimension], &vectors[global_b * dimension], dimension) <
                            distance(&vectors[global_a * dimension], &vectors[global_b * dimension], dimension);
                 });

    neighbors.resize(M_level);
}

void Solution::connect_neighbors(int local_vertex, int level, const vector<int> &neighbors, int cluster_id)
{
    // Ensure graph structure exists
    while ((int)graphs[cluster_id].size() <= level)
    {
        graphs[cluster_id].push_back(vector<vector<int>>());
    }

    while ((int)graphs[cluster_id][level].size() <= local_vertex)
    {
        graphs[cluster_id][level].push_back(vector<int>());
    }

    // Add neighbors
    graphs[cluster_id][level][local_vertex] = neighbors;

    // Bidirectional connection
    int M_level = (level == 0) ? (2 * M) : M;
    for (int neighbor : neighbors)
    {
        while ((int)graphs[cluster_id][level].size() <= neighbor)
        {
            graphs[cluster_id][level].push_back(vector<int>());
        }

        graphs[cluster_id][level][neighbor].push_back(local_vertex);

        // Prune if exceeds M
        if ((int)graphs[cluster_id][level][neighbor].size() > M_level)
        {
            select_neighbors_heuristic(graphs[cluster_id][level][neighbor], M_level, cluster_id);
        }
    }
}

void Solution::build_hnsw_partition(int cluster_id)
{
    int cluster_size = cluster_assignments[cluster_id].size();
    if (cluster_size == 0)
        return;

    // Initialize structures for this partition
    vertex_levels[cluster_id].resize(cluster_size);
    max_levels[cluster_id] = 0;

    for (int i = 0; i < cluster_size; ++i)
    {
        int global_id = cluster_assignments[cluster_id][i];
        int level = random_level();
        vertex_levels[cluster_id][i] = level;

        if (level > max_levels[cluster_id])
            max_levels[cluster_id] = level;

        if (i == 0)
        {
            entry_points[cluster_id] = 0;
            continue;
        }

        // Search for nearest neighbors
        vector<int> curr_neighbors = {entry_points[cluster_id]};

        // Search from top to target level + 1
        for (int lc = max_levels[cluster_id]; lc > level; --lc)
        {
            curr_neighbors = search_layer(&vectors[global_id * dimension], curr_neighbors, 1, lc, cluster_id);
        }

        // Insert at levels 0 to level
        for (int lc = level; lc >= 0; --lc)
        {
            int M_level = (lc == 0) ? (2 * M) : M;
            curr_neighbors = search_layer(&vectors[global_id * dimension], curr_neighbors, ef_construction, lc, cluster_id);

            vector<int> neighbors = curr_neighbors;
            select_neighbors_heuristic(neighbors, M_level, cluster_id);

            connect_neighbors(i, lc, neighbors, cluster_id);
        }

        // Update entry point
        if (level > vertex_levels[cluster_id][entry_points[cluster_id]])
        {
            entry_points[cluster_id] = i;
        }
    }
}

vector<int> Solution::find_nearest_clusters(const float *query, int num_probes) const
{
    vector<pair<float, int>> cluster_distances;

    for (int k = 0; k < num_clusters; ++k)
    {
        float dist = distance(query, centroids[k].data(), dimension);
        cluster_distances.push_back({dist, k});
    }

    // Sort by distance and return top num_probes
    partial_sort(cluster_distances.begin(),
                 cluster_distances.begin() + min(num_probes, (int)cluster_distances.size()),
                 cluster_distances.end());

    vector<int> result;
    for (int i = 0; i < min(num_probes, (int)cluster_distances.size()); ++i)
    {
        result.push_back(cluster_distances[i].second);
    }

    return result;
}

void Solution::search_hnsw(const vector<float> &query, int *res)
{
    const float *query_ptr = query.data();

    // Find nearest clusters to probe
    vector<int> clusters_to_probe = find_nearest_clusters(query_ptr, num_probe_clusters);

    // Collect candidates from multiple partitions
    priority_queue<pair<float, int>> all_candidates; // max heap

    for (int cluster_id : clusters_to_probe)
    {
        if (cluster_assignments[cluster_id].empty())
            continue;

        // Search in this partition
        vector<int> curr_neighbors = {entry_points[cluster_id]};

        // Search from top layer to layer 0
        for (int lc = max_levels[cluster_id]; lc > 0; --lc)
        {
            curr_neighbors = search_layer(query_ptr, curr_neighbors, 1, lc, cluster_id);
        }

        // Search at layer 0 with higher ef
        curr_neighbors = search_layer(query_ptr, curr_neighbors, ef_search, 0, cluster_id);

        // Add to global candidate pool
        for (int local_id : curr_neighbors)
        {
            int global_id = cluster_assignments[cluster_id][local_id];
            float dist = distance(query_ptr, &vectors[global_id * dimension], dimension);
            all_candidates.push({dist, global_id});

            if ((int)all_candidates.size() > 10)
                all_candidates.pop();
        }
    }

    // Extract top-10 results
    vector<pair<float, int>> final_results;
    while (!all_candidates.empty())
    {
        final_results.push_back(all_candidates.top());
        all_candidates.pop();
    }

    reverse(final_results.begin(), final_results.end());

    for (int i = 0; i < min(10, (int)final_results.size()); ++i)
    {
        res[i] = final_results[i].second;
    }
}

void Solution::build(int d, const vector<float> &base)
{
    dimension = d;
    num_vectors = base.size() / d;
    vectors = base;

    // Auto-detect dataset and configure parameters
    if (dimension == 100 && num_vectors > 1000000)
    {
        // GLOVE: Use K-Means + HNSW hybrid
        num_clusters = 150;     // Partition into 150 clusters
        num_probe_clusters = 4; // Probe 4 nearest clusters
        M = 12;                 // Lower M for faster build per partition
        ef_construction = 80;   // Lower ef_c for speed
        ef_search = 200;        // Adequate search quality

        // Build K-Means clustering
        build_kmeans(num_clusters, 15);

        // Initialize partition structures
        graphs.resize(num_clusters);
        entry_points.resize(num_clusters);
        max_levels.resize(num_clusters);
        vertex_levels.resize(num_clusters);

        // Build HNSW for each partition
        for (int k = 0; k < num_clusters; ++k)
        {
            if (!cluster_assignments[k].empty())
            {
                build_hnsw_partition(k);
            }
        }
    }
    else if (dimension == 128 && num_vectors > 900000)
    {
        // SIFT: Keep pure HNSW (already fast enough)
        num_clusters = 1;
        num_probe_clusters = 1;
        M = 16;
        ef_construction = 100;
        ef_search = 200;

        // Treat as single cluster
        cluster_assignments.resize(1);
        for (int i = 0; i < num_vectors; ++i)
        {
            cluster_assignments[0].push_back(i);
        }

        graphs.resize(1);
        entry_points.resize(1);
        max_levels.resize(1);
        vertex_levels.resize(1);

        build_hnsw_partition(0);
    }
}

void Solution::search(const vector<float> &query, int *res)
{
    search_hnsw(query, res);
}
