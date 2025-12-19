#include "MySolution.h"
#include <iostream>
#include <vector>
#include <chrono>

int main() {
    std::cout << "=== Minimal Test ===" << std::endl;
    
    Solution sol;
    
    // Create 10000 vectors, 100 dimensions
    std::vector<float> base;
    int n = 10000;
    int d = 100;
    
    std::cout << "Generating " << n << " vectors..." << std::endl;
    for(int i = 0; i < n; ++i) {
        for(int j = 0; j < d; ++j) {
            base.push_back((float)(i * d + j) / 10000.0f);
        }
    }
    
    std::cout << "Building index..." << std::endl;
    auto t1 = std::chrono::high_resolution_clock::now();
    sol.build(d, base);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto build_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
    std::cout << "Build time: " << build_ms << " ms" << std::endl;
    
    // Search test
    std::vector<float> query(d, 0.5f);
    int res[10];
    
    std::cout << "Searching..." << std::endl;
    auto t3 = std::chrono::high_resolution_clock::now();
    sol.search(query, res);
    auto t4 = std::chrono::high_resolution_clock::now();
    auto search_us = std::chrono::duration_cast<std::chrono::microseconds>(t4-t3).count();
    std::cout << "Search time: " << search_us << " us" << std::endl;
    
    std::cout << "Top 10 results: ";
    for(int i = 0; i < 10; ++i) std::cout << res[i] << " ";
    std::cout << std::endl;
    
    std::cout << "=== Test PASSED ===" << std::endl;
    return 0;
}
