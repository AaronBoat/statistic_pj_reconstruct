// Quick GLOVE parameter validation test
// Test with small subset to verify parameter effectiveness

#include "MySolution.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <random>

using namespace std;

int main() {
    // Simulate GLOVE dataset characteristics
    const int dimension = 100;
    const int num_vectors = 5000;  // Small subset for quick test
    const int num_queries = 100;
    
    cout << "Quick GLOVE Parameter Test\n";
    cout << "===========================\n";
    cout << "Dataset: " << num_vectors << " vectors, " << dimension << " dim\n\n";
    
    // Generate random vectors (simulating GLOVE distribution)
    random_device rd;
    mt19937 gen(42);
    uniform_real_distribution<float> dis(-1.0f, 1.0f);
    
    vector<float> base_vectors;
    base_vectors.reserve(num_vectors * dimension);
    for (int i = 0; i < num_vectors * dimension; ++i) {
        base_vectors.push_back(dis(gen));
    }
    
    vector<vector<float>> queries;
    for (int i = 0; i < num_queries; ++i) {
        vector<float> q;
        for (int j = 0; j < dimension; ++j) {
            q.push_back(dis(gen));
        }
        queries.push_back(q);
    }
    
    // Test with GLOVE parameters (auto-detected)
    Solution solution;
    
    auto build_start = chrono::high_resolution_clock::now();
    solution.build(dimension, base_vectors);
    auto build_end = chrono::high_resolution_clock::now();
    auto build_time = chrono::duration_cast<chrono::milliseconds>(build_end - build_start).count();
    
    cout << "Build time: " << build_time << " ms\n";
    
    // Estimate full dataset build time
    // GLOVE has 1,192,514 vectors, test has 5,000
    // Build complexity: O(N * M * ef_construction)
    double scale_factor = 1192514.0 / num_vectors;
    double estimated_full_build_ms = build_time * scale_factor;
    double estimated_full_build_min = estimated_full_build_ms / 60000.0;
    
    cout << "Estimated full GLOVE build time: " 
         << fixed << setprecision(1) << estimated_full_build_min << " minutes\n\n";
    
    // Perform searches
    solution.reset_distance_computations();
    
    auto search_start = chrono::high_resolution_clock::now();
    for (const auto& query : queries) {
        int results[10];
        solution.search(query, results);
    }
    auto search_end = chrono::high_resolution_clock::now();
    auto search_time = chrono::duration_cast<chrono::milliseconds>(search_end - search_start).count();
    
    long long total_dist = solution.get_distance_computations();
    double avg_dist = (double)total_dist / num_queries;
    double avg_query_ms = (double)search_time / num_queries;
    
    cout << "Search Results:\n";
    cout << "  Total search time: " << search_time << " ms\n";
    cout << "  Avg query time: " << fixed << setprecision(3) << avg_query_ms << " ms\n";
    cout << "  Avg distance computations: " << fixed << setprecision(0) << avg_dist << "\n\n";
    
    // Estimate full dataset search performance
    // Distance computations scale with dataset size, but not linearly (due to HNSW)
    // Rough estimate: sqrt scaling for navigable small world
    double full_dataset_scale = sqrt(1192514.0 / num_vectors);
    double estimated_full_dist = avg_dist * full_dataset_scale;
    
    cout << "Estimated full GLOVE performance:\n";
    cout << "  Avg distance computations: " << fixed << setprecision(0) 
         << estimated_full_dist << " (target: ~20000)\n";
    
    cout << "\nParameter Assessment:\n";
    if (estimated_full_build_min < 30) {
        cout << "  ✓ Build time looks good (<30 min)\n";
    } else {
        cout << "  ⚠ Build time may exceed 30 min\n";
    }
    
    if (estimated_full_dist >= 18000 && estimated_full_dist <= 25000) {
        cout << "  ✓ Distance computations in target range\n";
    } else if (estimated_full_dist < 18000) {
        cout << "  ⚠ May need higher ef_search for better recall\n";
    } else {
        cout << "  ⚠ May need lower ef_search for faster queries\n";
    }
    
    return 0;
}
