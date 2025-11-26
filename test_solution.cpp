#include "MySolution.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <string>
#include <set>

using namespace std;

// Load base vectors from file (line-by-line format)
vector<float> load_base_vectors(const string &filename, int &dimension, int &num_vectors)
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

    cout << "Loading vectors..." << flush;
    int progress_counter = 0;

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

        // Determine dimension from first vector
        if (dimension == 0)
        {
            dimension = vec.size();
            cout << " (dimension: " << dimension << ")" << endl;
        }

        // Add vector data
        for (float v : vec)
        {
            vectors.push_back(v);
        }
        num_vectors++;

        // Progress indicator
        if (++progress_counter % 100000 == 0)
        {
            cout << "  Loaded " << num_vectors << " vectors..." << endl;
        }
    }

    file.close();
    return vectors;
}

// Load query vectors from file
vector<vector<float>> load_query_vectors(const string &filename, int dimension)
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

        // Skip metadata line (e.g., "100 128")
        if (first_line && vec.size() == 2 && vec[0] > 0 && vec[1] > 0 && vec[0] < 100000 && vec[1] < 1000)
        {
            first_line = false;
            continue;
        }
        first_line = false;

        // Validate dimension
        if (dimension > 0 && vec.size() != dimension)
        {
            cerr << "Query dimension mismatch! Expected " << dimension
                 << ", got " << vec.size() << " in line" << endl;
            continue;
        }

        queries.push_back(vec);
    }

    file.close();
    return queries;
}

// Load groundtruth from file
vector<vector<int>> load_groundtruth(const string &filename)
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

        // Skip metadata line if exists
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
double calculate_recall(const vector<vector<int>> &results, const vector<vector<int>> &groundtruth, int k)
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

int main(int argc, char *argv[])
{
    // Default to SIFT dataset
    string dataset_dir = "../data_o/data_o/sift";

    if (argc > 1)
    {
        dataset_dir = argv[1];
    }

    string base_file = dataset_dir + "/base.txt";
    string query_file = dataset_dir + "/query.txt";
    string groundtruth_file = dataset_dir + "/groundtruth.txt";

    cout << "Using dataset: " << dataset_dir << endl;
    cout << "Loading base vectors..." << endl;
    int dimension, num_vectors;
    vector<float> base_vectors = load_base_vectors(base_file, dimension, num_vectors);

    if (base_vectors.empty())
    {
        cerr << "Failed to load base vectors" << endl;
        return 1;
    }

    cout << "Loaded " << num_vectors << " vectors of dimension " << dimension << endl;

    // Build index
    Solution solution;
    auto build_start = chrono::high_resolution_clock::now();
    solution.build(dimension, base_vectors);
    auto build_end = chrono::high_resolution_clock::now();
    auto build_time = chrono::duration_cast<chrono::milliseconds>(build_end - build_start).count();

    cout << "\nBuild time: " << build_time << " ms" << endl;

    // Load and search queries
    cout << "\nLoading query vectors..." << endl;
    vector<vector<float>> queries = load_query_vectors(query_file, dimension);

    if (queries.empty())
    {
        cerr << "Failed to load queries" << endl;
        return 1;
    }

    cout << "Loaded " << queries.size() << " query vectors" << endl;

    // Load groundtruth
    cout << "\nLoading groundtruth..." << endl;
    vector<vector<int>> groundtruth = load_groundtruth(groundtruth_file);

    if (groundtruth.empty())
    {
        cerr << "Failed to load groundtruth (continuing without recall calculation)" << endl;
    }
    else
    {
        cout << "Loaded groundtruth for " << groundtruth.size() << " queries" << endl;
    }

    // Perform searches
    cout << "\nPerforming searches..." << endl;
    
    // Reset distance computation counter before searching
    solution.reset_distance_computations();
    
    auto search_start = chrono::high_resolution_clock::now();

    vector<vector<int>> all_results;
    for (size_t i = 0; i < queries.size(); ++i)
    {
        int results[10];
        solution.search(queries[i], results);

        vector<int> result_vec(results, results + 10);
        all_results.push_back(result_vec);

        if (i < 5) // Print first 5 results
        {
            cout << "Query " << i << " results: ";
            for (int j = 0; j < 10; ++j)
            {
                cout << results[j] << " ";
            }
            cout << endl;
        }
    }

    auto search_end = chrono::high_resolution_clock::now();
    auto search_time = chrono::duration_cast<chrono::milliseconds>(search_end - search_start).count();

    cout << "\nTotal search time: " << search_time << " ms" << endl;
    cout << "Average search time: " << fixed << setprecision(2)
         << (double)search_time / queries.size() << " ms" << endl;

    // Get distance computation statistics
    long long total_distance_computations = solution.get_distance_computations();
    cout << "Total distance computations: " << total_distance_computations << endl;
    cout << "Average distance computations per query: " << fixed << setprecision(2)
         << (double)total_distance_computations / queries.size() << endl;

    // Calculate recall
    if (!groundtruth.empty() && groundtruth.size() == all_results.size())
    {
        double recall_1 = calculate_recall(all_results, groundtruth, 1);
        double recall_10 = calculate_recall(all_results, groundtruth, 10);

        cout << "\nRecall@1:  " << fixed << setprecision(4) << recall_1 << endl;
        cout << "Recall@10: " << fixed << setprecision(4) << recall_10 << endl;
    }

    return 0;
}
