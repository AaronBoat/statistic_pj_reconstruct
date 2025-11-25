#include "MySolution.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

Solution::Solution()
{
    // HNSW parameters - optimized for SIFT dataset
    M = 16;                // Good balance of connections
    ef_construction = 200; // Good graph structure without excessive build time
    ef_search = 400;       // High recall search
    ml = 1.0 / log(2.0);
    max_level = 0;

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

inline float Solution::distance(const float *a, const float *b, int dim) const
{
    ++distance_computations;
    
    float sum = 0.0f;
    int i = 0;

    // Loop unrolling for better performance (8-way for better cache utilization)
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
    visited.reserve(ef * 2);  // Pre-allocate to reduce rehashing

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

        // More aggressive early termination
        if (W.size() >= ef)
        {
            float w_top = W.top().first;
            if (current_dist > w_top)
                break;
        }

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
    search_hnsw(query, res);
}

void Solution::search_hnsw(const vector<float> &query, int *res)
{
    if (entry_point.empty())
    {
        for (int i = 0; i < 10; ++i)
            res[i] = i;
        return;
    }
    // Note: distance_computations is accumulated across all searches
    // Reset in test program if needed for per-query statistics
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
