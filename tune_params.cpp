/*
 * Parameter tuning script for HNSW
 * Tests different combinations of M, ef_construction, ef_search
 */

#include "MySolution.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <string>
#include <set>
#include <vector>

using namespace std;

// Load base vectors from file (line-by-line format)
vector<float> load_base_vectors(const string& filename, int& dimension, int& num_vectors)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Failed to open file: " << filename << endl;
        return vector<float>();
    }
    
    string line;
    vector<float> vectors;
    num_vectors = 0;
    dimension = 0;
    
    while (getline(file, line))
    {
        if (line.empty())
            continue;
        
        istringstream iss(line);
        vector<float> vec;
        float val;
        
        while (iss >> val)
        {
            vec.push_back(val);
        }
        
        if (vec.empty())
            continue;
        
        if (dimension == 0)
        {
            dimension = vec.size();
        }
        
        for (float v : vec)
        {
            vectors.push_back(v);
        }
        num_vectors++;
    }
    
    file.close();
    return vectors;
}

// Load query vectors from file
vector<vector<float>> load_query_vectors(const string& filename, int dimension)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Failed to open file: " << filename << endl;
        return vector<vector<float>>();
    }
    
    string line;
    vector<vector<float>> queries;
    bool first_line = true;
    
    while (getline(file, line))
    {
        if (line.empty())
            continue;
        
        istringstream iss(line);
        vector<float> vec;
        float val;
        
        while (iss >> val)
        {
            vec.push_back(val);
        }
        
        if (vec.empty())
            continue;
        
        if (first_line && vec.size() == 2 && vec[0] > 0 && vec[1] > 0 && vec[0] < 100000 && vec[1] < 1000)
        {
            first_line = false;
            continue;
        }
        first_line = false;
        
        if (dimension > 0 && vec.size() != dimension)
        {
            continue;
        }
        
        queries.push_back(vec);
    }
    
    file.close();
    return queries;
}

// Load groundtruth from file
vector<vector<int>> load_groundtruth(const string& filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Failed to open file: " << filename << endl;
        return vector<vector<int>>();
    }
    
    string line;
    vector<vector<int>> groundtruth;
    bool first_line = true;
    
    while (getline(file, line))
    {
        if (line.empty())
            continue;
        
        istringstream iss(line);
        vector<int> vec;
        int val;
        
        while (iss >> val)
        {
            vec.push_back(val);
        }
        
        if (vec.empty())
            continue;
        
        if (first_line && vec.size() == 2 && vec[0] > 0 && vec[1] > 0 && vec[0] < 100000 && vec[1] < 1000)
        {
            first_line = false;
            continue;
        }
        first_line = false;
        
        groundtruth.push_back(vec);
    }
    
    file.close();
    return groundtruth;
}

// Calculate recall@K
double calculate_recall(const vector<vector<int>>& results, const vector<vector<int>>& groundtruth, int k)
{
    if (results.size() != groundtruth.size())
    {
        cerr << "Results and groundtruth size mismatch!" << endl;
        return 0.0;
    }
    
    int total_recall = 0;
    for (size_t i = 0; i < results.size(); ++i)
    {
        set<int> gt_set;
        for (int j = 0; j < min(k, (int)groundtruth[i].size()); ++j)
        {
            gt_set.insert(groundtruth[i][j]);
        }
        
        int hits = 0;
        for (int j = 0; j < min(k, (int)results[i].size()); ++j)
        {
            if (gt_set.count(results[i][j]) > 0)
            {
                hits++;
            }
        }
        total_recall += hits;
    }
    
    return (double)total_recall / (results.size() * k);
}

int main()
{
    string dataset_dir = "../data_o/data_o/sift_small";
    
    string base_file = dataset_dir + "/base.txt";
    string query_file = dataset_dir + "/query.txt";
    string groundtruth_file = dataset_dir + "/groundtruth.txt";
    
    cout << "Loading data from: " << dataset_dir << endl;
    
    int dimension, num_vectors;
    vector<float> base_vectors = load_base_vectors(base_file, dimension, num_vectors);
    cout << "Loaded " << num_vectors << " vectors of dimension " << dimension << endl;
    
    vector<vector<float>> queries = load_query_vectors(query_file, dimension);
    cout << "Loaded " << queries.size() << " queries" << endl;
    
    vector<vector<int>> groundtruth = load_groundtruth(groundtruth_file);
    cout << "Loaded " << groundtruth.size() << " groundtruths" << endl;
    
    // Parameter combinations to test
    vector<int> M_values = {12, 16, 24, 32};
    vector<int> ef_construction_values = {100, 200, 400};
    vector<int> ef_search_values = {50, 100, 200, 400};
    
    cout << "\nTesting parameter combinations:\n" << endl;
    cout << "M\tef_c\tef_s\tBuild(ms)\tSearch(ms)\tR@1\tR@10" << endl;
    cout << "================================================================" << endl;
    
    for (int M : M_values)
    {
        for (int ef_c : ef_construction_values)
        {
            for (int ef_s : ef_search_values)
            {
                // Build index with current parameters
                Solution solution;
                solution.set_parameters(M, ef_c, ef_s);
                
                auto build_start = chrono::high_resolution_clock::now();
                solution.build(dimension, base_vectors);
                auto build_end = chrono::high_resolution_clock::now();
                auto build_time = chrono::duration_cast<chrono::milliseconds>(build_end - build_start).count();
                
                // Perform searches
                auto search_start = chrono::high_resolution_clock::now();
                vector<vector<int>> all_results;
                
                for (size_t i = 0; i < queries.size(); ++i)
                {
                    int results[10];
                    solution.search(queries[i], results);
                    all_results.push_back(vector<int>(results, results + 10));
                }
                
                auto search_end = chrono::high_resolution_clock::now();
                auto search_time = chrono::duration_cast<chrono::milliseconds>(search_end - search_start).count();
                
                // Calculate recall
                double recall_1 = calculate_recall(all_results, groundtruth, 1);
                double recall_10 = calculate_recall(all_results, groundtruth, 10);
                
                cout << M << "\t" << ef_c << "\t" << ef_s << "\t" 
                     << build_time << "\t\t" << search_time << "\t\t"
                     << fixed << setprecision(4) << recall_1 << "\t" << recall_10 << endl;
            }
        }
    }
    
    return 0;
}
