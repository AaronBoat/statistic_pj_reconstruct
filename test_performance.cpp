#include "MySolution.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

using namespace std;

int main()
{
    cout << "=== Performance Test ===" << endl;
    cout << "Testing distance computation optimization..." << endl
         << endl;

    // Create small test data
    int dim = 128;
    int num_vectors = 1000;
    int num_queries = 100;

    vector<float> base;
    for (int i = 0; i < num_vectors * dim; ++i)
    {
        base.push_back((float)(rand() % 1000) / 1000.0f);
    }

    vector<vector<float>> queries;
    for (int i = 0; i < num_queries; ++i)
    {
        vector<float> q;
        for (int j = 0; j < dim; ++j)
        {
            q.push_back((float)(rand() % 1000) / 1000.0f);
        }
        queries.push_back(q);
    }

    // Build index
    Solution solution;
    cout << "Building index with " << num_vectors << " vectors..." << endl;
    auto build_start = chrono::high_resolution_clock::now();
    solution.build(dim, base);
    auto build_end = chrono::high_resolution_clock::now();
    auto build_time = chrono::duration_cast<chrono::milliseconds>(build_end - build_start).count();
    cout << "Build time: " << build_time << " ms" << endl
         << endl;

    // Search
    cout << "Searching " << num_queries << " queries..." << endl;
    solution.reset_distance_computations();

    auto search_start = chrono::high_resolution_clock::now();
    for (int i = 0; i < num_queries; ++i)
    {
        int results[10];
        solution.search(queries[i], results);
    }
    auto search_end = chrono::high_resolution_clock::now();
    auto search_time = chrono::duration_cast<chrono::milliseconds>(search_end - search_start).count();

    long long total_dist = solution.get_distance_computations();

    cout << "\n=== Results ===" << endl;
    cout << "Total search time: " << search_time << " ms" << endl;
    cout << "Average search time: " << fixed << setprecision(3)
         << (double)search_time / num_queries << " ms" << endl;
    cout << "Total distance computations: " << total_dist << endl;
    cout << "Average distance computations per query: " << fixed << setprecision(1)
         << (double)total_dist / num_queries << endl;

    cout << "\n=== Optimizations Applied ===" << endl;
    cout << "✓ 8-way loop unrolling in distance calculation" << endl;
    cout << "✓ Inline distance function" << endl;
    cout << "✓ Pre-allocated hash set memory" << endl;
    cout << "✓ More aggressive early termination" << endl;
    cout << "✓ Distance computation statistics tracking" << endl;

    return 0;
}
