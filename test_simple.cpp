#include "MySolution.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

// Simple test with synthetic data
int main()
{
    cout << "=== Simple HNSW Test ===" << endl;
    
    // Create small test dataset
    int dim = 4;
    int num_vectors = 100;
    
    vector<float> base;
    for (int i = 0; i < num_vectors; ++i)
    {
        for (int j = 0; j < dim; ++j)
        {
            base.push_back((float)(i * dim + j));
        }
    }
    
    cout << "Created " << num_vectors << " vectors of dimension " << dim << endl;
    
    // Build index
    Solution solution;
    cout << "\nBuilding index..." << endl;
    
    try {
        solution.build(dim, base);
        cout << "Index built successfully!" << endl;
    } catch (const exception& e) {
        cout << "Error during build: " << e.what() << endl;
        return 1;
    }
    
    // Test search with first vector
    cout << "\nTesting search..." << endl;
    vector<float> query = {0.0f, 1.0f, 2.0f, 3.0f}; // Should match first vector
    int results[10];
    
    try {
        solution.search(query, results);
    } catch (const exception& e) {
        cout << "Error during search: " << e.what() << endl;
        return 1;
    }
    
    cout << "Query: [" << query[0];
    for (int i = 1; i < dim; ++i)
    {
        cout << ", " << query[i];
    }
    cout << "]" << endl;
    
    cout << "Top 10 results: ";
    for (int i = 0; i < 10; ++i)
    {
        cout << results[i];
        if (i < 9) cout << ", ";
    }
    cout << endl;
    
    // Verify first result should be 0
    if (results[0] == 0)
    {
        cout << "\n✓ Test PASSED: First result is correct!" << endl;
        return 0;
    }
    else
    {
        cout << "\n✗ Test FAILED: Expected first result to be 0, got " 
             << results[0] << endl;
        return 1;
    }
}
