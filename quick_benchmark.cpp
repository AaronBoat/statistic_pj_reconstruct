#include "MySolution.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <cstring>
#include <set>

using namespace std;

// Load fvecs file
vector<vector<float>> load_fvecs(const string& filename, int max_count = -1) {
    ifstream input(filename, ios::binary);
    if (!input) {
        cerr << "Cannot open: " << filename << endl;
        return {};
    }
    
    vector<vector<float>> data;
    int dim;
    int count = 0;
    
    while (input.read(reinterpret_cast<char*>(&dim), sizeof(int))) {
        vector<float> vec(dim);
        input.read(reinterpret_cast<char*>(vec.data()), dim * sizeof(float));
        data.push_back(vec);
        
        count++;
        if (max_count > 0 && count >= max_count) break;
    }
    
    return data;
}

// Load ivecs file
vector<vector<int>> load_ivecs(const string& filename) {
    ifstream input(filename, ios::binary);
    if (!input) {
        cerr << "Cannot open: " << filename << endl;
        return {};
    }
    
    vector<vector<int>> data;
    int dim;
    
    while (input.read(reinterpret_cast<char*>(&dim), sizeof(int))) {
        vector<int> vec(dim);
        input.read(reinterpret_cast<char*>(vec.data()), dim * sizeof(int));
        data.push_back(vec);
    }
    
    return data;
}

double calculate_recall(const vector<vector<int>>& results, 
                       const vector<vector<int>>& groundtruth, int k) {
    int total_correct = 0;
    int total_queries = results.size();
    
    for (size_t i = 0; i < results.size(); i++) {
        set<int> gt_set;
        for (int j = 0; j < k && j < groundtruth[i].size(); j++) {
            gt_set.insert(groundtruth[i][j]);
        }
        
        int correct = 0;
        for (int j = 0; j < k && j < results[i].size(); j++) {
            if (gt_set.count(results[i][j])) {
                correct++;
            }
        }
        total_correct += correct;
    }
    
    return (double)total_correct / (total_queries * k);
}

int main(int argc, char* argv[]) {
    string base_path = (argc > 1) ? argv[1] : "../data_o/data_o/sift_small";
    
    cout << "=== Quick Benchmark ===" << endl;
    cout << "Dataset: " << base_path << endl << endl;
    
    // Load data
    cout << "Loading base vectors..." << flush;
    auto base = load_fvecs(base_path + "/sift_base.fvecs");
    cout << " " << base.size() << " vectors" << endl;
    
    cout << "Loading queries..." << flush;
    auto queries = load_fvecs(base_path + "/sift_query.fvecs");
    cout << " " << queries.size() << " queries" << endl;
    
    cout << "Loading groundtruth..." << flush;
    auto groundtruth = load_ivecs(base_path + "/sift_groundtruth.ivecs");
    cout << " loaded" << endl << endl;
    
    if (base.empty() || queries.empty() || groundtruth.empty()) {
        cerr << "Failed to load data!" << endl;
        return 1;
    }
    
    // Build index
    Solution solution;
    int dimension = base[0].size();
    
    cout << "Building index..." << flush;
    auto build_start = chrono::high_resolution_clock::now();
    
    // Flatten base vectors
    vector<float> flat_base;
    for (const auto& vec : base) {
        flat_base.insert(flat_base.end(), vec.begin(), vec.end());
    }
    solution.build(dimension, flat_base);
    
    auto build_end = chrono::high_resolution_clock::now();
    double build_time = chrono::duration<double>(build_end - build_start).count();
    cout << " Done!" << endl;
    cout << "Build time: " << build_time << " seconds" << endl << endl;
    
    // Search
    cout << "Searching..." << flush;
    vector<vector<int>> results(queries.size(), vector<int>(10));
    
    auto search_start = chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < queries.size(); i++) {
        solution.search(queries[i], results[i].data());
    }
    
    auto search_end = chrono::high_resolution_clock::now();
    double search_time = chrono::duration<double>(search_end - search_start).count();
    double avg_query_time = (search_time * 1000.0) / queries.size();
    
    cout << " Done!" << endl;
    cout << "Total search time: " << search_time << " seconds" << endl;
    cout << "Average query time: " << avg_query_time << " ms" << endl << endl;
    
    // Calculate recall
    double recall_1 = calculate_recall(results, groundtruth, 1);
    double recall_10 = calculate_recall(results, groundtruth, 10);
    
    cout << "=== Results ===" << endl;
    cout << "Recall@1:  " << recall_1 << endl;
    cout << "Recall@10: " << recall_10 << endl << endl;
    
    cout << "=== Summary ===" << endl;
    cout << "Build time: " << build_time << " s" << endl;
    cout << "Query time: " << avg_query_time << " ms" << endl;
    cout << "Recall@10:  " << recall_10 << endl;
    
    return 0;
}
