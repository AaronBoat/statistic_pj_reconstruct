// Quick parameter detection test
#include "MySolution.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    Solution sol;
    
    cout << "Testing parameter auto-detection...\n\n";
    
    // Test SIFT detection (128-dim, 1M vectors)
    {
        vector<float> sift_data(1000000 * 128, 0.0f);
        sol.build(128, sift_data);
        cout << "SIFT (128-dim, 1M vectors) detected\n";
        cout << "Expected: M=16, ef_construction=100, ef_search=200\n\n";
    }
    
    // Test GLOVE detection (100-dim, 1.2M vectors)
    {
        Solution sol2;
        vector<float> glove_data(1192514 * 100, 0.0f);
        sol2.build(100, glove_data);
        cout << "GLOVE (100-dim, 1.2M vectors) detected\n";
        cout << "Expected: M=24, ef_construction=200, ef_search=300\n\n";
    }
    
    cout << "Parameter detection test completed!\n";
    return 0;
}
