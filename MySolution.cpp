#include "MySolution.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

Solution::Solution()
{
    // Algorithm auto-selection based on dataset
    use_ivf = false; // Will be set in build()

    // HNSW parameters - optimized for high recall and reasonable build time
    M = 16;                // Good balance of connections
    ef_construction = 200; // Good graph structure without excessive build time
    ef_search = 400;       // High recall search
    ml = 1.0 / log(2.0);
    max_level = 0;

    // IVF parameters - optimized for GLOVE-like datasets
    nlist = 4096; // number of clusters (sqrt(N) is a good heuristic)
    nprobe = 64;  // search top 64 clusters

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

float Solution::distance(const float *a, const float *b, int dim) const
{
    float sum = 0.0f;
    int i = 0;

    // Loop unrolling for better performance
    for (; i + 4 <= dim; i += 4)
    {
        float diff0 = a[i] - b[i];
        float diff1 = a[i + 1] - b[i + 1];
        float diff2 = a[i + 2] - b[i + 2];
        float diff3 = a[i + 3] - b[i + 3];
        sum += diff0 * diff0 + diff1 * diff1 + diff2 * diff2 + diff3 * diff3;
    }

    // Handle remaining elements
    for (; i < dim; ++i)
    {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum; // L2 squared distance
}

int Solution::random_level()
{
    float r = (float)rng() / (float)rng.max();
    return (int)(-log(r) * ml);
}

vector<int> Solution::search_layer(const float *query, const vector<int> &entry_points,
                                   int ef, int level) const
{
    unordered_set<int> visited;

    // Priority queue: (distance, vertex_id)
    // Min heap for candidates (to get closest one first)
    auto cmp_min = [](const pair<float, int> &a, const pair<float, int> &b)
    {
        return a.first > b.first;
    };
    priority_queue<pair<float, int>, vector<pair<float, int>>, decltype(cmp_min)> candidates(cmp_min);

    // Max heap for results (W) - keep ef closest, top() is farthest among them
    auto cmp_max = [](const pair<float, int> &a, const pair<float, int> &b)
    {
        return a.first < b.first;
    };
    priority_queue<pair<float, int>, vector<pair<float, int>>, decltype(cmp_max)> W(cmp_max);

    // Initialize with entry points
    for (int ep : entry_points)
    {
        float dist = distance(query, &vectors[ep * dimension], dimension);
        candidates.push({dist, ep});
        W.push({dist, ep});
        visited.insert(ep);
    }

    while (!candidates.empty())
    {
        auto current = candidates.top();
        candidates.pop();
        float current_dist = current.first;
        int current_id = current.second;

        // Check if we should stop
        // W.top() is the farthest among the ef closest points
        if (W.size() >= ef && current_dist > W.top().first)
            break;

        // Check neighbors
        if (level < graph.size() && current_id < graph[level].size())
        {
            for (int neighbor : graph[level][current_id])
            {
                if (visited.find(neighbor) == visited.end())
                {
                    visited.insert(neighbor);
                    float dist = distance(query, &vectors[neighbor * dimension], dimension);

                    // Add if W is not full or if this is closer than the farthest in W
                    if (W.size() < ef || dist < W.top().first)
                    {
                        candidates.push({dist, neighbor});
                        W.push({dist, neighbor});

                        if (W.size() > ef)
                            W.pop(); // Remove the farthest
                    }
                }
            }
        }
    }

    // Extract results
    vector<int> result;
    while (!W.empty())
    {
        result.push_back(W.top().second);
        W.pop();
    }
    reverse(result.begin(), result.end());
    return result;
}

void Solution::select_neighbors_heuristic(vector<int> &neighbors, int M_level)
{
    if (neighbors.size() <= M_level)
        return;

    // Simple but effective: keep M_level closest neighbors
    // The neighbors are already sorted by distance from search_layer
    // This is fast and works well for most cases
    neighbors.resize(M_level);
}

void Solution::connect_neighbors(int vertex, int level, const vector<int> &neighbors)
{
    // Ensure graph size
    while (graph.size() <= level)
    {
        graph.push_back(vector<vector<int>>());
    }

    while (graph[level].size() <= vertex)
    {
        graph[level].push_back(vector<int>());
    }

    // Add connections
    graph[level][vertex] = neighbors;

    // Add bidirectional connections
    int M_level = (level == 0) ? (2 * M) : M;
    const float *vertex_vec = &vectors[vertex * dimension];

    for (int neighbor : neighbors)
    {
        while (graph[level].size() <= neighbor)
        {
            graph[level].push_back(vector<int>());
        }

        auto &neighbor_connections = graph[level][neighbor];
        if (find(neighbor_connections.begin(), neighbor_connections.end(), vertex) == neighbor_connections.end())
        {
            neighbor_connections.push_back(vertex);

            // Prune if necessary - only if exceeds limit significantly
            if (neighbor_connections.size() > M_level * 1.2)
            {
                // Sort by distance to neighbor before pruning
                const float *neighbor_vec = &vectors[neighbor * dimension];
                vector<pair<float, int>> dist_pairs;
                dist_pairs.reserve(neighbor_connections.size());

                for (int conn : neighbor_connections)
                {
                    float dist = distance(neighbor_vec, &vectors[conn * dimension], dimension);
                    dist_pairs.push_back({dist, conn});
                }

                // Partial sort - only sort what we need
                nth_element(dist_pairs.begin(),
                            dist_pairs.begin() + M_level,
                            dist_pairs.end());
                sort(dist_pairs.begin(), dist_pairs.begin() + M_level);

                // Extract sorted connections
                neighbor_connections.clear();
                neighbor_connections.reserve(M_level);
                for (size_t i = 0; i < M_level; ++i)
                {
                    neighbor_connections.push_back(dist_pairs[i].second);
                }
            }
        }
    }
}

void Solution::build(int d, const vector<float> &base)
{
    dimension = d;
    num_vectors = base.size() / d;
    vectors = base;

    vertex_level.resize(num_vectors);
    entry_point.clear();

    for (int i = 0; i < num_vectors; ++i)
    {
        int level = random_level();
        vertex_level[i] = level;

        if (level > max_level)
            max_level = level;

        if (entry_point.empty())
        {
            entry_point.push_back(i);
            continue;
        }

        // Search for nearest neighbors
        vector<int> curr_neighbors = entry_point;

        // Search from top to target level + 1
        for (int lc = max_level; lc > level; --lc)
        {
            curr_neighbors = search_layer(&vectors[i * dimension], curr_neighbors, 1, lc);
        }

        // Insert at levels 0 to level
        for (int lc = level; lc >= 0; --lc)
        {
            int M_level = (lc == 0) ? (2 * M) : M;
            curr_neighbors = search_layer(&vectors[i * dimension], curr_neighbors, ef_construction, lc);

            // Select M neighbors
            vector<int> neighbors = curr_neighbors;
            select_neighbors_heuristic(neighbors, M_level);

            // Connect to graph
            connect_neighbors(i, lc, neighbors);
        }

        // Update entry point
        if (level > vertex_level[entry_point[0]])
        {
            entry_point[0] = i;
        }
    }
}

void Solution::search(const vector<float> &query, int *res)
{
    if (use_ivf)
    {
        search_ivf(query.data(), res);
    }
    else
    {
        search_hnsw(query, res);
    }
}

void Solution::search_hnsw(const vector<float> &query, int *res)
{
    if (entry_point.empty())
    {
        for (int i = 0; i < 10; ++i)
            res[i] = i;
        return;
    }

    const float *query_ptr = query.data();
    vector<int> curr_neighbors = entry_point;

    // Search from top level to level 1
    for (int lc = max_level; lc > 0; --lc)
    {
        curr_neighbors = search_layer(query_ptr, curr_neighbors, 1, lc);
    }

    // Search at layer 0 with ef_search
    curr_neighbors = search_layer(query_ptr, curr_neighbors, ef_search, 0);

    // Return top 10 results
    for (int i = 0; i < 10 && i < curr_neighbors.size(); ++i)
    {
        res[i] = curr_neighbors[i];
    }

    // Fill remaining slots if needed
    for (int i = curr_neighbors.size(); i < 10; ++i)
    {
        res[i] = 0;
    }
}

// ============================================================================
// IVF (Inverted File Index) Implementation
// Better for GLOVE-like datasets with uniform distribution
// ============================================================================

void Solution::build_ivf()
{
    // Step 1: K-means clustering to create nlist centroids
    kmeans_clustering();

    // Step 2: Assign each vector to nearest centroid
    inverted_lists.resize(nlist);

    for (int i = 0; i < num_vectors; ++i)
    {
        int cluster_id = assign_to_nearest_centroid(&vectors[i * dimension]);
        inverted_lists[cluster_id].push_back(i);
    }
}

void Solution::kmeans_clustering()
{
    centroids.resize(nlist, vector<float>(dimension, 0.0f));

    // Initialize centroids using K-means++ for better initial distribution
    vector<int> chosen_indices;
    uniform_int_distribution<int> dist(0, num_vectors - 1);

    // Choose first centroid randomly
    int first_idx = dist(rng);
    for (int d = 0; d < dimension; ++d)
    {
        centroids[0][d] = vectors[first_idx * dimension + d];
    }
    chosen_indices.push_back(first_idx);

    // Choose remaining centroids with K-means++ strategy
    for (int c = 1; c < nlist; ++c)
    {
        vector<float> min_distances(num_vectors, numeric_limits<float>::max());

        // Compute minimum distance to any existing centroid
        for (int i = 0; i < num_vectors; ++i)
        {
            float dist = distance(&vectors[i * dimension], centroids[c - 1].data(), dimension);
            min_distances[i] = min(min_distances[i], dist);
        }

        // Choose next centroid with probability proportional to distance squared
        float sum_dist = 0.0f;
        for (float d : min_distances)
            sum_dist += d;

        float rand_val = (float)rng() / rng.max() * sum_dist;
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

        for (int d = 0; d < dimension; ++d)
        {
            centroids[c][d] = vectors[next_idx * dimension + d];
        }
        chosen_indices.push_back(next_idx);
    }

    // Run K-means iterations
    int max_iterations = 20;
    for (int iter = 0; iter < max_iterations; ++iter)
    {
        // Assignment step
        vector<vector<int>> clusters(nlist);
        for (int i = 0; i < num_vectors; ++i)
        {
            int nearest = assign_to_nearest_centroid(&vectors[i * dimension]);
            clusters[nearest].push_back(i);
        }

        // Update step
        bool converged = true;
        for (int c = 0; c < nlist; ++c)
        {
            if (clusters[c].empty())
                continue;

            vector<float> new_centroid(dimension, 0.0f);
            for (int idx : clusters[c])
            {
                for (int d = 0; d < dimension; ++d)
                {
                    new_centroid[d] += vectors[idx * dimension + d];
                }
            }

            for (int d = 0; d < dimension; ++d)
            {
                new_centroid[d] /= clusters[c].size();
                if (abs(new_centroid[d] - centroids[c][d]) > 1e-6)
                {
                    converged = false;
                }
                centroids[c][d] = new_centroid[d];
            }
        }

        if (converged && iter > 5)
        {
            break;
        }
    }
}

int Solution::assign_to_nearest_centroid(const float *vec) const
{
    int nearest = 0;
    float min_dist = distance(vec, centroids[0].data(), dimension);

    for (int c = 1; c < nlist; ++c)
    {
        float dist = distance(vec, centroids[c].data(), dimension);
        if (dist < min_dist)
        {
            min_dist = dist;
            nearest = c;
        }
    }

    return nearest;
}

void Solution::search_ivf(const float *query, int *res) const
{
    // Find nprobe nearest centroids
    vector<pair<float, int>> centroid_distances;
    for (int c = 0; c < nlist; ++c)
    {
        float dist = distance(query, centroids[c].data(), dimension);
        centroid_distances.push_back({dist, c});
    }

    // Partial sort to get top nprobe clusters
    int actual_nprobe = min(nprobe, nlist);
    nth_element(centroid_distances.begin(),
                centroid_distances.begin() + actual_nprobe,
                centroid_distances.end());

    // Search vectors in selected clusters
    vector<pair<float, int>> candidates;

    for (int i = 0; i < actual_nprobe; ++i)
    {
        int cluster_id = centroid_distances[i].second;

        for (int vec_id : inverted_lists[cluster_id])
        {
            float dist = distance(query, &vectors[vec_id * dimension], dimension);
            candidates.push_back({dist, vec_id});
        }
    }

    // Get top 10 results
    int k = min(10, (int)candidates.size());
    if (k > 0)
    {
        nth_element(candidates.begin(),
                    candidates.begin() + k,
                    candidates.end());
        sort(candidates.begin(), candidates.begin() + k);

        for (int i = 0; i < k; ++i)
        {
            res[i] = candidates[i].second;
        }
    }

    // Fill remaining slots
    for (int i = k; i < 10; ++i)
    {
        res[i] = 0;
    }
}
