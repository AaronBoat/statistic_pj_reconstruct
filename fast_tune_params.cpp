#include "MySolution.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

using namespace std;

// Load base vectors from file (line-by-line format)
vector<float> load_base(const string &filename, int &d, int &n)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Failed to open: " << filename << endl;
        return vector<float>();
    }

    string line;
    vector<float> vectors;
    n = 0;
    d = 0;

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        istringstream iss(line);
        vector<float> vec;
        float val;

        while (iss >> val)
            vec.push_back(val);

        if (vec.empty())
            continue;

        if (d == 0)
            d = vec.size();

        if ((int)vec.size() != d)
        {
            cerr << "Dimension mismatch at line " << n + 1 << endl;
            continue;
        }

        vectors.insert(vectors.end(), vec.begin(), vec.end());
        n++;
    }

    return vectors;
}

// Load query vectors (same format)
vector<float> load_query(const string &filename, int &d, int &n)
{
    return load_base(filename, d, n);
}

// Load groundtruth
vector<vector<int>> load_groundtruth(const string &filename, int n)
{
    ifstream in(filename);
    vector<vector<int>> gt(n, vector<int>(10));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < 10; ++j)
            in >> gt[i][j];
    return gt;
}

// Calculate recall
double calculate_recall(const vector<vector<int>> &results, const vector<vector<int>> &gt)
{
    int correct = 0;
    int total = results.size() * 10;
    for (size_t i = 0; i < results.size(); ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            for (int k = 0; k < 10; ++k)
            {
                if (results[i][j] == gt[i][k])
                {
                    correct++;
                    break;
                }
            }
        }
    }
    return (double)correct / total;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " <data_folder> [build_new_graph]" << endl;
        cout << "  build_new_graph: 0=load cached, 1=build new (default: 0)" << endl;
        return 1;
    }

    string data_folder = argv[1];
    bool build_new = (argc > 2) ? (atoi(argv[2]) == 1) : false;
    string graph_cache_file = data_folder + "_graph_cache.bin";

    // Load data
    cout << "Loading data..." << endl;
    int d, n_base, n_query;
    vector<float> base = load_base(data_folder + "/base.txt", d, n_base);
    vector<float> query = load_query(data_folder + "/query.txt", d, n_query);
    vector<vector<int>> gt = load_groundtruth(data_folder + "/groundtruth.txt", n_query);

    Solution sol;

    if (!build_new && ifstream(graph_cache_file).good())
    {
        cout << "Loading cached graph from: " << graph_cache_file << endl;
        auto start = chrono::high_resolution_clock::now();
        
        if (sol.load_graph(graph_cache_file))
        {
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
            cout << "Graph loaded in " << duration.count() << " ms" << endl;
            cout << "Dimensions: " << d << ", Vectors: " << n_base << endl;
        }
        else
        {
            cout << "Failed to load graph, building new one..." << endl;
            build_new = true;
        }
    }
    else
    {
        build_new = true;
    }

    if (build_new)
    {
        cout << "Building new graph..." << endl;
        cout << "Dataset: " << n_base << " vectors, " << d << " dimensions" << endl;
        auto start = chrono::high_resolution_clock::now();
        sol.build(d, base);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        cout << "Build time: " << duration.count() << " ms (" << duration.count() / 1000.0 / 60.0 << " min)" << endl;

        cout << "Saving graph cache to: " << graph_cache_file << endl;
        if (sol.save_graph(graph_cache_file))
        {
            cout << "Graph cached successfully!" << endl;
        }
        else
        {
            cout << "Failed to save graph cache" << endl;
        }
    }

    // Fast parameter tuning
    cout << "\n========== Fast Parameter Tuning ==========" << endl;
    cout << "Testing different ef_search values with cached graph...\n" << endl;

    // Test different ef_search values
    vector<int> ef_search_values = {1500, 2000, 2200, 2400, 2600, 2800, 3000, 3200, 3500};
    
    cout << setw(12) << "ef_search" << setw(15) << "Recall@10" << setw(18) << "Search Time(ms)" 
         << setw(20) << "Avg Dist Comps" << endl;
    cout << string(65, '-') << endl;

    for (int ef_s : ef_search_values)
    {
        sol.set_parameters(20, 165, ef_s); // Keep M=20, ef_c=165 fixed
        sol.reset_distance_computations();

        vector<vector<int>> results(n_query, vector<int>(10));
        
        auto start = chrono::high_resolution_clock::now();
        for (int i = 0; i < n_query; ++i)
        {
            vector<float> q(query.begin() + i * d, query.begin() + (i + 1) * d);
            sol.search(q, results[i].data());
        }
        auto end = chrono::high_resolution_clock::now();

        double recall = calculate_recall(results, gt);
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        double avg_dist_comps = (double)sol.get_distance_computations() / n_query;

        cout << setw(12) << ef_s 
             << setw(15) << fixed << setprecision(4) << recall
             << setw(18) << duration.count()
             << setw(20) << fixed << setprecision(0) << avg_dist_comps
             << endl;

        // Stop if we've exceeded reasonable search time
        if (duration.count() > 5000)
        {
            cout << "Search time too high, stopping parameter sweep..." << endl;
            break;
        }
    }

    cout << "\n========== Testing with NGT Adaptive Search ==========" << endl;
    cout << "Using gamma=0.19 (NGT-inspired threshold)\n" << endl;
    
    // Note: Would need to modify search to use adaptive version
    cout << "(Adaptive search requires code modification to enable)" << endl;

    return 0;
}
