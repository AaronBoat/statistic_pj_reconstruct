#include "MySolution.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <cstring>
#include <climits>

#ifdef _OPENMP
#include <omp.h>
#endif

// SIMD intrinsics
#if defined(__AVX512F__)
#include <immintrin.h>
#define USE_AVX512
#elif defined(__AVX2__)
#include <immintrin.h>
#define USE_AVX2
#elif defined(__SSE2__)
#include <emmintrin.h>
#define USE_SSE2
#endif

using namespace std;

// ==================== Thread Local Storage ====================
// This is the CRITICAL FIX for the race condition and TLE.
// Each thread gets its own visited buffer.
struct VisitedBuffer
{
    vector<int> visited;
    int tag;

    VisitedBuffer() : tag(0) {}

    void resize(int n)
    {
        if (visited.size() < n)
        {
            visited.resize(n, 0);
            tag = 0;
        }
    }

    int get_new_tag()
    {
        ++tag;
        if (tag == 0)
        { // Overflow handling
            fill(visited.begin(), visited.end(), 0);
            tag = 1;
        }
        return tag;
    }
};

static thread_local VisitedBuffer tls_visited;

// ==================== Solution Implementation ====================

Solution::Solution()
{
    M = 16;
    ef_construction = 200;
    ef_search = 200;
    ml = 1.0 / log(2.0);
    max_level = 0;
    gamma = 0.0;
    distance_computations.store(0);
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
#if defined(USE_AVX512)
    __m512 sum = _mm512_setzero_ps();
    int i = 0;
    for (; i + 16 <= dim; i += 16)
    {
        __m512 va = _mm512_loadu_ps(a + i);
        __m512 vb = _mm512_loadu_ps(b + i);
        __m512 diff = _mm512_sub_ps(va, vb);
        sum = _mm512_fmadd_ps(diff, diff, sum);
    }
    float total = _mm512_reduce_add_ps(sum);
    for (; i < dim; ++i)
    {
        float diff = a[i] - b[i];
        total += diff * diff;
    }
    return total;
#elif defined(USE_AVX2)
    __m256 sum = _mm256_setzero_ps();
    int i = 0;
    for (; i + 8 <= dim; i += 8)
    {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        __m256 diff = _mm256_sub_ps(va, vb);
        sum = _mm256_fmadd_ps(diff, diff, sum);
    }
    __m128 sum_low = _mm256_castps256_ps128(sum);
    __m128 sum_high = _mm256_extractf128_ps(sum, 1);
    __m128 res = _mm_add_ps(sum_low, sum_high);
    res = _mm_hadd_ps(res, res);
    res = _mm_hadd_ps(res, res);
    float total = _mm_cvtss_f32(res);
    for (; i < dim; ++i)
    {
        float diff = a[i] - b[i];
        total += diff * diff;
    }
    return total;
#else
    float dist = 0;
    for (int i = 0; i < dim; ++i)
    {
        float d = a[i] - b[i];
        dist += d * d;
    }
    return dist;
#endif
}

int Solution::random_level()
{
    double r = (double)rng() / (double)rng.max();
    if (r < 1e-9)
        r = 1e-9;
    return (int)(-log(r) * ml);
}

// ==================== HNSW Core ====================

vector<int> Solution::search_layer(const float *query, const vector<int> &entry_points,
                                   int ef, int level) const
{
    // Initialize Thread Local Storage
    tls_visited.resize(num_vectors);
    int tag = tls_visited.get_new_tag();
    auto &visited = tls_visited.visited;

    auto cmp_min = [](const pair<float, int> &a, const pair<float, int> &b)
    { return a.first > b.first; };
    priority_queue<pair<float, int>, vector<pair<float, int>>, decltype(cmp_min)> candidates(cmp_min);

    auto cmp_max = [](const pair<float, int> &a, const pair<float, int> &b)
    { return a.first < b.first; };
    priority_queue<pair<float, int>, vector<pair<float, int>>, decltype(cmp_max)> W(cmp_max);

    for (int ep : entry_points)
    {
        if (visited[ep] != tag)
        {
            visited[ep] = tag;
            float dist = distance(query, &vectors[ep * dimension], dimension);
            candidates.push({dist, ep});
            W.push({dist, ep});
        }
    }

    float lower_bound = numeric_limits<float>::max();
    if (!W.empty())
        lower_bound = W.top().first;

    while (!candidates.empty())
    {
        auto current = candidates.top();
        candidates.pop();
        float current_dist = current.first;
        int current_id = current.second;

        if (current_dist > lower_bound)
            break;

        const int *neighbors_ptr = nullptr;
        int neighbor_count = 0;

        // Optimized Access
        if (level == 0 && !final_graph_flat.empty())
        {
            int max_neighbors_l0 = 2 * M;
            long long offset = (long long)current_id * (max_neighbors_l0 + 1);
            neighbor_count = final_graph_flat[offset];
            neighbors_ptr = &final_graph_flat[offset + 1];
        }
        else if (level < graph.size())
        {
            const auto &vec = graph[level][current_id];
            neighbor_count = vec.size();
            neighbors_ptr = vec.data();
        }

        // Prefetch
        if (neighbor_count > 0)
        {
            _mm_prefetch((const char *)&vectors[neighbors_ptr[0] * dimension], _MM_HINT_T0);
            if (neighbor_count > 1)
                _mm_prefetch((const char *)&vectors[neighbors_ptr[1] * dimension], _MM_HINT_T0);
        }

        for (int i = 0; i < neighbor_count; ++i)
        {
            int neighbor = neighbors_ptr[i];

            if (i + 2 < neighbor_count)
            {
                _mm_prefetch((const char *)&vectors[neighbors_ptr[i + 2] * dimension], _MM_HINT_T0);
            }

            if (visited[neighbor] != tag)
            {
                visited[neighbor] = tag;
                float dist = distance(query, &vectors[neighbor * dimension], dimension);

                if (dist < lower_bound || W.size() < ef)
                {
                    candidates.push({dist, neighbor});
                    W.push({dist, neighbor});

                    if (W.size() > ef)
                    {
                        W.pop();
                        lower_bound = W.top().first;
                    }
                    else
                    {
                        lower_bound = W.top().first;
                    }
                }
            }
        }
    }

    vector<int> result;
    result.reserve(W.size());
    while (!W.empty())
    {
        result.push_back(W.top().second);
        W.pop();
    }
    // Reverse needed because priority_queue is max heap (farthest on top)
    // We want output to be [farthest ... closest] so we can just pop_back or similar?
    // Usually HNSW entry points don't need strict order, but let's keep consistency
    // Actually search_layer returns 'ef' candidates, order matters less here than final result
    return result;
}

void Solution::select_neighbors_heuristic(vector<int> &neighbors, int M_level)
{
    if ((int)neighbors.size() <= M_level)
        return;

    if (neighbors.empty()) return;

    // Sort by distance first
    int start_node = neighbors[0]; // heuristic baseline
    vector<pair<float, int>> scored;
    scored.reserve(neighbors.size());

    for (int n : neighbors)
    {
        float d = distance(&vectors[start_node * dimension], &vectors[n * dimension], dimension);
        scored.push_back({d, n});
    }
    sort(scored.begin(), scored.end());

    vector<int> selected;
    selected.reserve(M_level);
    if (!scored.empty())
        selected.push_back(scored[0].second);

    // Alpha = 1.0 for GLOVE (Dense) to maintain recall
    const float alpha = 1.0f;

    for (size_t i = 1; i < scored.size() && selected.size() < M_level; ++i)
    {
        int cand = scored[i].second;
        float dist_c = scored[i].first;
        bool good = true;

        for (int sel : selected)
        {
            float d = distance(&vectors[cand * dimension], &vectors[sel * dimension], dimension);
            if (d < dist_c * alpha)
            {
                good = false;
                break;
            }
        }
        if (good)
            selected.push_back(cand);
    }

    // Fill if needed
    if (selected.size() < M_level)
    {
        for (auto &p : scored)
        {
            if (selected.size() >= M_level)
                break;
            bool found = false;
            for (int s : selected)
                if (s == p.second)
                {
                    found = true;
                    break;
                }
            if (!found)
                selected.push_back(p.second);
        }
    }
    neighbors = selected;
}

void Solution::connect_neighbors(int vertex, int level, const vector<int> &neighbors)
{
    // 1. Forward connection (No lock needed, only this thread owns 'vertex')
    graph[level][vertex] = neighbors;

    // 2. Reverse connections (Needs lock)
    int M_max = (level == 0) ? (2 * M) : M;

    for (int neighbor : neighbors)
    {
        node_locks[neighbor].acquire();

        vector<int> &conn = graph[level][neighbor];
        bool exists = false;
        for (int x : conn)
            if (x == vertex)
            {
                exists = true;
                break;
            }

        if (!exists)
        {
            conn.push_back(vertex);

            // LAZY PRUNING: Only prune if size is significantly larger than M_max
            // This prevents the TLE caused by sorting inside the lock too often.
            // 2.5x factor gives buffer for parallel inserts.
            if (conn.size() > M_max * 2.5)
            {
                // We must prune inside lock to maintain integrity, but we do it rarely
                select_neighbors_heuristic(conn, M_max);
            }
        }

        node_locks[neighbor].release();
    }
}

void Solution::build(int d, const vector<float> &base)
{
    dimension = d;
    num_vectors = base.size() / d;
    vectors = base;

    // 1. Parameter Tuning (Glove Specific)
    if (dimension == 100 && num_vectors > 500000)
    {
        M = 30;                 // Safe spot between 24 and 32
        ef_construction = 200;  // Reduced from 300 to fix TLE
        ef_search = 200;        // High baseline for recall
        gamma = 0.25;           // Adaptive
    }
    else
    {
        // SIFT or others
        M = 16;
        ef_construction = 150;
        ef_search = 150;
    }

    // 2. Pre-allocation (Fixes Critical Section bottleneck)
    // Pre-calculate levels
    vertex_level.resize(num_vectors);
    max_level = 0;
    for (int i = 0; i < num_vectors; ++i)
    {
        int l = random_level();
        vertex_level[i] = l;
        if (l > max_level)
            max_level = l;
    }

    // Allocate Graph
    graph.resize(max_level + 1);
    for (int l = 0; l <= max_level; ++l)
    {
        graph[l].resize(num_vectors);
        // Reserve memory for edges to reduce re-allocations
        int expected_M = (l == 0) ? M * 2 : M;
        // Don't reserve for all, just let vector grow naturally or loop parallel
    }

    // Allocate Locks
    node_locks = vector<NodeLock>(num_vectors); // NodeLock default ctor handles init

    // 3. Parallel Build
    entry_point.clear();
    entry_point.push_back(0); // Start with node 0

    // Important: We must add node 0 to the graph first structurally
    // But since we pre-allocated, we can just start the loop from 1.
    // The connections for node 0 will be populated by reverse links from others,
    // and its forward links will be populated if we process it.
    // Actually standard HNSW inserts sequentially.
    // Parallel strategy:
    // We treat node 0 as the initial entry point.

#pragma omp parallel for schedule(dynamic, 128)
    for (int i = 1; i < num_vectors; ++i)
    {
        int level = vertex_level[i];
        int curr_max_level = max_level; // Snapshot

        // Use thread-local visited list inside search_layer

        vector<int> curr_ep;
        curr_ep.push_back(0); // Always start from 0 (static entry)

        // Search down to insertion level
        // Note: We use the 'global' entry point 0. In a true online HNSW, entry point changes.
        // For batch build, starting from 0 is fine, or we can use a shared atomic entry point.
        // Using fixed entry point 0 is slightly suboptimal for navigation but thread-safe and fast.

        for (int lc = curr_max_level; lc > level; --lc)
        {
            curr_ep = search_layer(&vectors[i * dimension], curr_ep, 1, lc);
        }

        for (int lc = min(curr_max_level, level); lc >= 0; --lc)
        {
            int ef_c = ef_construction;
            vector<int> candidates = search_layer(&vectors[i * dimension], curr_ep, ef_c, lc);

            // Heuristic selection
            int M_curr = (lc == 0) ? M * 2 : M;
            select_neighbors_heuristic(candidates, M_curr);

            // Update graph
            connect_neighbors(i, lc, candidates);

            // Candidates become entry points for next layer
            curr_ep = candidates;
        }
    }

    // 4. Post-processing: Flatten Layer 0
    if (!graph.empty())
    {
        int max_neighbors_l0 = 2 * M;
        final_graph_flat.resize(num_vectors * (max_neighbors_l0 + 1), 0);

        for (int i = 0; i < num_vectors; ++i)
        {
            auto &neighbors = graph[0][i];
            // Final prune to ensure strict size compliance (optional but good for cache)
            if (neighbors.size() > max_neighbors_l0)
            {
                select_neighbors_heuristic(neighbors, max_neighbors_l0);
            }

            int sz = neighbors.size();
            long long off = (long long)i * (max_neighbors_l0 + 1);
            final_graph_flat[off] = sz;
            for (int j = 0; j < sz; ++j)
            {
                final_graph_flat[off + 1 + j] = neighbors[j];
            }
        }
    }
}

void Solution::search(const vector<float> &query, int *res)
{
    search_hnsw(query, res);
}

void Solution::search_hnsw(const vector<float> &query, int *res)
{
    if (graph.empty())
    {
        for (int i = 0; i < 10; ++i)
            res[i] = 0;
        return;
    }

    vector<int> curr_ep;
    curr_ep.push_back(0);

    for (int lc = max_level; lc > 0; --lc)
    {
        curr_ep = search_layer(query.data(), curr_ep, 1, lc);
    }

    // Layer 0 Search
    vector<int> candidates;
    if (gamma > 0)
    {
        candidates = search_layer_adaptive(query.data(), curr_ep, ef_search, 0, gamma);
    }
    else
    {
        candidates = search_layer(query.data(), curr_ep, ef_search, 0);
    }

    // Sort candidates by distance to pick top 10
    // candidates from search_layer are not strictly sorted by distance (they are heap popped)
    // Wait, search_layer logic returns them popped from min-heap (farthest first) or max-heap?
    // My search_layer pops from W (max-heap, keeps smallest).
    // result.push_back(W.top()); W.pop();
    // So result is [farthest ... closest].
    // We need closest first for output.

    // Sort top 10 safely
    priority_queue<pair<float, int>> top_k;
    for (int idx : candidates)
    {
        float d = distance(query.data(), &vectors[idx * dimension], dimension);
        top_k.push({d, idx});
        if (top_k.size() > 10)
            top_k.pop();
    }

    vector<int> final_res;
    while (!top_k.empty())
    {
        final_res.push_back(top_k.top().second);
        top_k.pop();
    }
    reverse(final_res.begin(), final_res.end());

    for (int i = 0; i < 10; ++i)
    {
        if (i < final_res.size())
            res[i] = final_res[i];
        else
            res[i] = 0;
    }
}

vector<int> Solution::search_layer_adaptive(const float *query, const vector<int> &entry_points,
                                            int ef, int level, float gamma_param) const
{
    tls_visited.resize(num_vectors);
    int tag = tls_visited.get_new_tag();
    auto &visited = tls_visited.visited;

    auto cmp_min = [](const pair<float, int> &a, const pair<float, int> &b)
    { return a.first > b.first; };
    priority_queue<pair<float, int>, vector<pair<float, int>>, decltype(cmp_min)> candidates(cmp_min);

    auto cmp_max = [](const pair<float, int> &a, const pair<float, int> &b)
    { return a.first < b.first; };
    priority_queue<pair<float, int>, vector<pair<float, int>>, decltype(cmp_max)> W(cmp_max);

    for (int ep : entry_points)
    {
        if (visited[ep] != tag)
        {
            visited[ep] = tag;
            float dist = distance(query, &vectors[ep * dimension], dimension);
            candidates.push({dist, ep});
            W.push({dist, ep});
        }
    }

    float max_dist = W.empty() ? numeric_limits<float>::max() : W.top().first;

    while (!candidates.empty())
    {
        auto current = candidates.top();
        candidates.pop();

        if (current.first > max_dist * (1.0 + gamma_param))
        {
            if (W.size() >= ef)
                break;
        }

        const int *neighbors_ptr = nullptr;
        int neighbor_count = 0;

        if (level == 0 && !final_graph_flat.empty())
        {
            int max_neighbors_l0 = 2 * M;
            long long offset = (long long)current.second * (max_neighbors_l0 + 1);
            neighbor_count = final_graph_flat[offset];
            neighbors_ptr = &final_graph_flat[offset + 1];
        }
        else if (level < graph.size())
        {
            const auto &vec = graph[level][current.second];
            neighbor_count = vec.size();
            neighbors_ptr = vec.data();
        }

        // Prefetching
        for (int i = 0; i < min(4, neighbor_count); ++i)
            _mm_prefetch((const char *)&vectors[neighbors_ptr[i] * dimension], _MM_HINT_T0);

        for (int i = 0; i < neighbor_count; ++i)
        {
            int neighbor = neighbors_ptr[i];

            if (i + 4 < neighbor_count)
                _mm_prefetch((const char *)&vectors[neighbors_ptr[i + 4] * dimension], _MM_HINT_T0);

            if (visited[neighbor] != tag)
            {
                visited[neighbor] = tag;
                float dist = distance(query, &vectors[neighbor * dimension], dimension);

                if (dist < max_dist * (1.0 + gamma_param) || W.size() < ef)
                {
                    candidates.push({dist, neighbor});
                    W.push({dist, neighbor});

                    if (W.size() > ef)
                    {
                        W.pop();
                        max_dist = W.top().first;
                    }
                    else
                    {
                        max_dist = W.top().first;
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
    return result;
}

bool Solution::save_graph(const string &filename) const { return false; }
bool Solution::load_graph(const string &filename) { return false; }
