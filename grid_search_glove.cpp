// Grid Search for GLOVE parameter optimization
#include "MySolution.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace std;

// Load vectors from file
vector<float> load_vectors(const string &filename, int &dimension, int &num_vectors)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Failed to open " << filename << endl;
        exit(1);
    }

    file >> num_vectors >> dimension;
    vector<float> data(num_vectors * dimension);

    for (int i = 0; i < num_vectors * dimension; ++i)
    {
        file >> data[i];
    }

    file.close();
    return data;
}

// Load groundtruth
vector<vector<int>> load_groundtruth(const string &filename, int num_queries)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Failed to open " << filename << endl;
        exit(1);
    }

    vector<vector<int>> groundtruth(num_queries, vector<int>(10));
    for (int i = 0; i < num_queries; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            file >> groundtruth[i][j];
        }
    }

    file.close();
    return groundtruth;
}

// Calculate recall
double calculate_recall(const vector<vector<int>> &results,
                        const vector<vector<int>> &groundtruth)
{
    int total_correct = 0;
    int total = results.size() * 10;

    for (size_t i = 0; i < results.size(); ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            for (int k = 0; k < 10; ++k)
            {
                if (results[i][j] == groundtruth[i][k])
                {
                    total_correct++;
                    break;
                }
            }
        }
    }

    return (double)total_correct / total;
}

struct GridResult
{
    int M;
    int ef_construction;
    int ef_search;
    double build_time_sec;
    double recall;
    double avg_search_ms;
    long long avg_distance_comp;
};

int main()
{
    cout << "========================================" << endl;
    cout << "  GLOVE Grid Search Parameter Tuning" << endl;
    cout << "========================================" << endl;
    cout << endl;

    string dataset_path = "../data_o/data_o/glove";

    // Load data
    cout << "Loading base vectors..." << endl;
    int dimension, num_vectors;
    vector<float> base = load_vectors(dataset_path + "/base.fvecs", dimension, num_vectors);

    // If fvecs format failed, try txt format
    if (num_vectors == 0)
    {
        base = load_vectors(dataset_path + "/base.txt", dimension, num_vectors);
    }

    cout << "Loaded " << num_vectors << " vectors of dimension " << dimension << endl;

    cout << "Loading query vectors..." << endl;
    int query_dim, num_queries;
    vector<float> queries = load_vectors(dataset_path + "/query.fvecs", query_dim, num_queries);

    if (num_queries == 0)
    {
        queries = load_vectors(dataset_path + "/query.txt", query_dim, num_queries);
    }

    cout << "Loaded " << num_queries << " queries" << endl;

    cout << "Loading groundtruth..." << endl;
    vector<vector<int>> groundtruth = load_groundtruth(dataset_path + "/groundtruth.txt", num_queries);
    cout << endl;

    // Define parameter grid
    vector<int> M_values = {16, 18, 20, 22};
    vector<int> ef_construction_values = {150, 160, 170, 180};
    vector<int> ef_search_values = {1500, 2000, 2500, 3000};

    vector<GridResult> results;

    // Open CSV file
    ofstream csv_file("glove_grid_search_results.csv");
    csv_file << "M,ef_construction,ef_search,build_time_sec,recall,avg_search_ms,avg_dist_comp,meets_98%" << endl;

    int total_combinations = M_values.size() * ef_construction_values.size() * ef_search_values.size();
    int current = 0;

    cout << "Starting grid search with " << total_combinations << " combinations..." << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << endl;

    for (int M : M_values)
    {
        for (int ef_c : ef_construction_values)
        {
            for (int ef_s : ef_search_values)
            {
                current++;
                cout << "[" << current << "/" << total_combinations << "] ";
                cout << "Testing M=" << M << ", ef_c=" << ef_c << ", ef_s=" << ef_s << " ... " << flush;

                Solution solution;
                solution.set_parameters(M, ef_c, ef_s);

                // Build index
                auto build_start = chrono::high_resolution_clock::now();
                solution.build(dimension, base);
                auto build_end = chrono::high_resolution_clock::now();
                double build_time = chrono::duration<double>(build_end - build_start).count();

                // Check if build time exceeds limit
                if (build_time > 1800) // 30 minutes
                {
                    cout << "SKIP (build time " << (int)(build_time / 60) << "min > 30min)" << endl;
                    continue;
                }

                // Search
                vector<vector<int>> search_results(num_queries, vector<int>(10));
                solution.reset_distance_computations();

                auto search_start = chrono::high_resolution_clock::now();
                for (int i = 0; i < num_queries; ++i)
                {
                    vector<float> query(queries.begin() + i * dimension,
                                        queries.begin() + (i + 1) * dimension);
                    solution.search(query, search_results[i].data());
                }
                auto search_end = chrono::high_resolution_clock::now();

                double total_search_ms = chrono::duration<double, milli>(search_end - search_start).count();
                double avg_search_ms = total_search_ms / num_queries;
                long long total_dist_comp = solution.get_distance_computations();
                long long avg_dist_comp = total_dist_comp / num_queries;

                // Calculate recall
                double recall = calculate_recall(search_results, groundtruth);

                GridResult result;
                result.M = M;
                result.ef_construction = ef_c;
                result.ef_search = ef_s;
                result.build_time_sec = build_time;
                result.recall = recall;
                result.avg_search_ms = avg_search_ms;
                result.avg_distance_comp = avg_dist_comp;

                results.push_back(result);

                // Output to console
                cout << "Build: " << (int)build_time << "s, ";
                cout << "Recall: " << fixed << setprecision(2) << recall * 100 << "%, ";
                cout << "Search: " << fixed << setprecision(2) << avg_search_ms << "ms";

                if (recall >= 0.98)
                {
                    cout << " ✓" << endl;
                }
                else
                {
                    cout << endl;
                }

                // Write to CSV
                csv_file << M << "," << ef_c << "," << ef_s << ","
                         << (int)build_time << ","
                         << fixed << setprecision(4) << recall << ","
                         << fixed << setprecision(2) << avg_search_ms << ","
                         << avg_dist_comp << ","
                         << (recall >= 0.98 ? "YES" : "NO") << endl;
                csv_file.flush();
            }
        }
    }

    csv_file.close();

    cout << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Grid search complete! Results saved to glove_grid_search_results.csv" << endl;
    cout << endl;

    // Find best configurations
    cout << "Top 5 configurations meeting 98% recall:" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    // Filter and sort by search time
    vector<GridResult> valid_results;
    for (const auto &r : results)
    {
        if (r.recall >= 0.98 && r.build_time_sec <= 1800)
        {
            valid_results.push_back(r);
        }
    }

    sort(valid_results.begin(), valid_results.end(),
         [](const GridResult &a, const GridResult &b)
         {
             return a.avg_search_ms < b.avg_search_ms;
         });

    int count = 0;
    for (const auto &r : valid_results)
    {
        if (count >= 5)
            break;
        cout << ++count << ". M=" << r.M << ", ef_c=" << r.ef_construction
             << ", ef_s=" << r.ef_search << endl;
        cout << "   Build: " << (int)r.build_time_sec << "s, "
             << "Recall: " << fixed << setprecision(2) << r.recall * 100 << "%, "
             << "Search: " << fixed << setprecision(2) << r.avg_search_ms << "ms, "
             << "Dist: " << r.avg_distance_comp << endl;
    }

    if (valid_results.empty())
    {
        cout << "No configurations met the 98% recall requirement!" << endl;
        cout << endl;
        cout << "Best recall achieved:" << endl;
        auto best = max_element(results.begin(), results.end(),
                                [](const GridResult &a, const GridResult &b)
                                {
                                    return a.recall < b.recall;
                                });
        if (best != results.end())
        {
            cout << "M=" << best->M << ", ef_c=" << best->ef_construction
                 << ", ef_s=" << best->ef_search << endl;
            cout << "Recall: " << fixed << setprecision(2) << best->recall * 100 << "%" << endl;
        }
    }

    cout << endl;
    return 0;
}
