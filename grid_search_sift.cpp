/*
 * Grid Search Parameter Tuning for SIFT Dataset
 * Systematically tests parameter combinations and outputs detailed results
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

// Load base vectors from file
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

// Load query vectors
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

        // Skip metadata line
        if (first_line && vec.size() == 2)
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

// Load groundtruth
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

        // Skip metadata line
        if (first_line && vec.size() == 2)
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

struct ParamResult
{
    int M;
    int ef_construction;
    int ef_search;
    long long build_time_ms;
    long long search_time_ms;
    double avg_search_ms;
    double recall_1;
    double recall_10;
    double score; // Composite score
};

int main(int argc, char *argv[])
{
    // Parse command line arguments
    string dataset_dir = "../data_o/data_o/sift_small";
    if (argc > 1)
    {
        dataset_dir = argv[1];
    }

    string base_file = dataset_dir + "/base.txt";
    string query_file = dataset_dir + "/query.txt";
    string groundtruth_file = dataset_dir + "/groundtruth.txt";

    cout << "============================================================" << endl;
    cout << "  SIFT Grid Search Parameter Tuning" << endl;
    cout << "============================================================" << endl;
    cout << "Dataset: " << dataset_dir << endl
         << endl;

    // Load data
    cout << "Loading data..." << endl;
    int dimension, num_vectors;
    vector<float> base_vectors = load_base_vectors(base_file, dimension, num_vectors);
    if (base_vectors.empty())
    {
        cerr << "Failed to load base vectors!" << endl;
        return 1;
    }
    cout << "  Base vectors: " << num_vectors << " x " << dimension << "D" << endl;

    vector<vector<float>> queries = load_query_vectors(query_file, dimension);
    cout << "  Query vectors: " << queries.size() << endl;

    vector<vector<int>> groundtruth = load_groundtruth(groundtruth_file);
    cout << "  Groundtruth: " << groundtruth.size() << endl
         << endl;

    // Define parameter grid
    vector<int> M_values = {12, 16, 20, 24};
    vector<int> ef_construction_values = {100, 150, 200, 300};
    vector<int> ef_search_values = {50, 100, 200, 300, 400, 500};

    int total_combinations = M_values.size() * ef_construction_values.size() * ef_search_values.size();

    cout << "Parameter Grid:" << endl;
    cout << "  M: ";
    for (int m : M_values)
        cout << m << " ";
    cout << endl;
    cout << "  ef_construction: ";
    for (int ef : ef_construction_values)
        cout << ef << " ";
    cout << endl;
    cout << "  ef_search: ";
    for (int ef : ef_search_values)
        cout << ef << " ";
    cout << endl;
    cout << "  Total combinations: " << total_combinations << endl
         << endl;

    // Results storage
    vector<ParamResult> results;

    cout << "Starting grid search..." << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << setw(3) << "M" << " | "
         << setw(5) << "ef_c" << " | "
         << setw(5) << "ef_s" << " | "
         << setw(8) << "Build(s)" << " | "
         << setw(8) << "Query(ms)" << " | "
         << setw(7) << "R@1" << " | "
         << setw(7) << "R@10" << " | "
         << setw(7) << "Score" << endl;
    cout << "------------------------------------------------------------" << endl;

    int test_count = 0;

    for (int M : M_values)
    {
        for (int ef_c : ef_construction_values)
        {
            for (int ef_s : ef_search_values)
            {
                test_count++;

                // Build index
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
                    int res[10];
                    solution.search(queries[i], res);
                    all_results.push_back(vector<int>(res, res + 10));
                }

                auto search_end = chrono::high_resolution_clock::now();
                auto search_time = chrono::duration_cast<chrono::milliseconds>(search_end - search_start).count();
                double avg_search = (double)search_time / queries.size();

                // Calculate recall
                double recall_1 = calculate_recall(all_results, groundtruth, 1);
                double recall_10 = calculate_recall(all_results, groundtruth, 10);

                // Calculate composite score (higher is better)
                // Score = recall@10 * 100 - build_time_penalty - search_time_penalty
                double score = recall_10 * 100.0 - (build_time / 1000.0) * 0.01 - avg_search * 0.1;

                // Store result
                ParamResult result;
                result.M = M;
                result.ef_construction = ef_c;
                result.ef_search = ef_s;
                result.build_time_ms = build_time;
                result.search_time_ms = search_time;
                result.avg_search_ms = avg_search;
                result.recall_1 = recall_1;
                result.recall_10 = recall_10;
                result.score = score;
                results.push_back(result);

                // Print progress
                cout << setw(3) << M << " | "
                     << setw(5) << ef_c << " | "
                     << setw(5) << ef_s << " | "
                     << setw(8) << fixed << setprecision(2) << (build_time / 1000.0) << " | "
                     << setw(8) << fixed << setprecision(2) << avg_search << " | "
                     << setw(7) << fixed << setprecision(4) << recall_1 << " | "
                     << setw(7) << fixed << setprecision(4) << recall_10 << " | "
                     << setw(7) << fixed << setprecision(2) << score
                     << " [" << test_count << "/" << total_combinations << "]" << endl;
            }
        }
    }

    cout << "============================================================" << endl
         << endl;

    // Find best configurations
    auto best_recall = results[0];
    auto best_speed = results[0];
    auto best_score = results[0];

    for (const auto &r : results)
    {
        if (r.recall_10 > best_recall.recall_10)
            best_recall = r;
        if (r.avg_search_ms < best_speed.avg_search_ms)
            best_speed = r;
        if (r.score > best_score.score)
            best_score = r;
    }

    cout << "Best Configurations:" << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << "Best Recall@10:" << endl;
    cout << "  M=" << best_recall.M
         << ", ef_construction=" << best_recall.ef_construction
         << ", ef_search=" << best_recall.ef_search << endl;
    cout << "  Recall@10: " << fixed << setprecision(4) << best_recall.recall_10
         << ", Query time: " << fixed << setprecision(2) << best_recall.avg_search_ms << "ms" << endl
         << endl;

    cout << "Best Query Speed:" << endl;
    cout << "  M=" << best_speed.M
         << ", ef_construction=" << best_speed.ef_construction
         << ", ef_search=" << best_speed.ef_search << endl;
    cout << "  Query time: " << fixed << setprecision(2) << best_speed.avg_search_ms << "ms"
         << ", Recall@10: " << fixed << setprecision(4) << best_speed.recall_10 << endl
         << endl;

    cout << "Best Overall Score:" << endl;
    cout << "  M=" << best_score.M
         << ", ef_construction=" << best_score.ef_construction
         << ", ef_search=" << best_score.ef_search << endl;
    cout << "  Score: " << fixed << setprecision(2) << best_score.score
         << ", Recall@10: " << fixed << setprecision(4) << best_score.recall_10
         << ", Query time: " << fixed << setprecision(2) << best_score.avg_search_ms << "ms" << endl
         << endl;

    // Save results to CSV
    string csv_filename = "sift_grid_search_results.csv";
    ofstream csv_file(csv_filename);
    if (csv_file.is_open())
    {
        csv_file << "M,ef_construction,ef_search,build_time_ms,search_time_ms,avg_search_ms,recall_1,recall_10,score" << endl;
        for (const auto &r : results)
        {
            csv_file << r.M << ","
                     << r.ef_construction << ","
                     << r.ef_search << ","
                     << r.build_time_ms << ","
                     << r.search_time_ms << ","
                     << fixed << setprecision(4) << r.avg_search_ms << ","
                     << fixed << setprecision(6) << r.recall_1 << ","
                     << fixed << setprecision(6) << r.recall_10 << ","
                     << fixed << setprecision(4) << r.score << endl;
        }
        csv_file.close();
        cout << "Results saved to: " << csv_filename << endl;
    }

    // Save detailed markdown report
    string md_filename = "sift_grid_search_report.md";
    ofstream md_file(md_filename);
    if (md_file.is_open())
    {
        md_file << "# SIFT Grid Search Results\n\n";
        md_file << "## Dataset Information\n";
        md_file << "- Vectors: " << num_vectors << "\n";
        md_file << "- Dimension: " << dimension << "\n";
        md_file << "- Queries: " << queries.size() << "\n\n";

        md_file << "## Parameter Grid\n";
        md_file << "- M: ";
        for (int m : M_values)
            md_file << m << " ";
        md_file << "\n- ef_construction: ";
        for (int ef : ef_construction_values)
            md_file << ef << " ";
        md_file << "\n- ef_search: ";
        for (int ef : ef_search_values)
            md_file << ef << " ";
        md_file << "\n- Total combinations: " << total_combinations << "\n\n";

        md_file << "## All Results\n\n";
        md_file << "| M | ef_c | ef_s | Build(s) | Query(ms) | R@1 | R@10 | Score |\n";
        md_file << "|---|------|------|----------|-----------|-----|------|-------|\n";
        for (const auto &r : results)
        {
            md_file << "| " << r.M
                    << " | " << r.ef_construction
                    << " | " << r.ef_search
                    << " | " << fixed << setprecision(2) << (r.build_time_ms / 1000.0)
                    << " | " << fixed << setprecision(2) << r.avg_search_ms
                    << " | " << fixed << setprecision(4) << r.recall_1
                    << " | " << fixed << setprecision(4) << r.recall_10
                    << " | " << fixed << setprecision(2) << r.score
                    << " |\n";
        }

        md_file << "\n## Best Configurations\n\n";
        md_file << "### Best Recall@10\n";
        md_file << "- M=" << best_recall.M
                << ", ef_construction=" << best_recall.ef_construction
                << ", ef_search=" << best_recall.ef_search << "\n";
        md_file << "- Recall@10: " << fixed << setprecision(4) << best_recall.recall_10 << "\n";
        md_file << "- Query time: " << fixed << setprecision(2) << best_recall.avg_search_ms << "ms\n\n";

        md_file << "### Best Query Speed\n";
        md_file << "- M=" << best_speed.M
                << ", ef_construction=" << best_speed.ef_construction
                << ", ef_search=" << best_speed.ef_search << "\n";
        md_file << "- Query time: " << fixed << setprecision(2) << best_speed.avg_search_ms << "ms\n";
        md_file << "- Recall@10: " << fixed << setprecision(4) << best_speed.recall_10 << "\n\n";

        md_file << "### Best Overall Score\n";
        md_file << "- M=" << best_score.M
                << ", ef_construction=" << best_score.ef_construction
                << ", ef_search=" << best_score.ef_search << "\n";
        md_file << "- Score: " << fixed << setprecision(2) << best_score.score << "\n";
        md_file << "- Recall@10: " << fixed << setprecision(4) << best_score.recall_10 << "\n";
        md_file << "- Query time: " << fixed << setprecision(2) << best_score.avg_search_ms << "ms\n";

        md_file.close();
        cout << "Detailed report saved to: " << md_filename << endl;
    }

    cout << "\nGrid search completed!" << endl;

    return 0;
}
