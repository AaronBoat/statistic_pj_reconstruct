#!/usr/bin/env python3
"""
Create a small subset of SIFT dataset for quick testing
"""

import os

def create_subset(input_file, output_file, num_lines):
    """Read first num_lines from input_file and write to output_file"""
    with open(input_file, 'r') as f:
        lines = []
        for i, line in enumerate(f):
            if i >= num_lines:
                break
            lines.append(line)
    
    with open(output_file, 'w') as f:
        f.writelines(lines)
    
    print(f"Created {output_file} with {len(lines)} lines")

# Create small dataset directory
small_dir = "../data_o/data_o/sift_small"
sift_dir = "../data_o/data_o/sift"

os.makedirs(small_dir, exist_ok=True)

# Create small base (10000 vectors)
create_subset(os.path.join(sift_dir, "base.txt"), 
              os.path.join(small_dir, "base.txt"), 
              10000)

# Copy query file (already small - 100 queries)
create_subset(os.path.join(sift_dir, "query.txt"), 
              os.path.join(small_dir, "query.txt"), 
              101)  # 100 queries + 1 metadata line

# Copy groundtruth file
create_subset(os.path.join(sift_dir, "groundtruth.txt"), 
              os.path.join(small_dir, "groundtruth.txt"), 
              101)  # 100 groundtruths + 1 metadata line

print(f"\nSmall dataset created in {small_dir}")
print("Run: .\\test_solution.exe ..\\data_o\\data_o\\sift_small")
