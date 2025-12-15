#!/usr/bin/env python3
"""
Fine-grained parameter tuning tool for HNSW optimization
Tests combinations of M, ef_construction, ef_search to find optimal config
"""

import subprocess
import re
import json
from datetime import datetime
import sys

# Fine-grained parameter grid
PARAM_GRID = {
    'M': [14, 16, 18, 20],
    'ef_construction': [120, 140, 150, 160, 170],
    'ef_search': [2000, 2200, 2400, 2600, 2800]
}

# Quick test mode (fewer combinations)
QUICK_GRID = {
    'M': [16, 18],
    'ef_construction': [140, 150, 160],
    'ef_search': [2200, 2400, 2600]
}

def modify_params(M, ef_c, ef_s):
    """Modify MySolution.cpp parameters"""
    with open('MySolution.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    modified = False
    for i, line in enumerate(lines):
        if 'GLOVE: Fine-tuned' in line or 'GLOVE: Test config' in line:
            # Found the GLOVE section, modify next 3 lines
            lines[i] = f'        // GLOVE: Test config M={M}, ef_c={ef_c}, ef_s={ef_s}\n'
            # Find and modify M, ef_construction, ef_search
            for j in range(i+1, min(i+10, len(lines))):
                if lines[j].strip().startswith('M ='):
                    lines[j] = f'        M = {M};\n'
                elif lines[j].strip().startswith('ef_construction ='):
                    lines[j] = f'        ef_construction = {ef_c};\n'
                elif lines[j].strip().startswith('ef_search ='):
                    lines[j] = f'        ef_search = {ef_s};\n'
            modified = True
            break
    
    if not modified:
        print("ERROR: Could not find GLOVE config section")
        return False
    
    with open('MySolution.cpp', 'w', encoding='utf-8') as f:
        f.writelines(lines)
    return True

def compile_code():
    """Compile with optimizations"""
    result = subprocess.run([
        'g++', '-std=c++11', '-O3', '-mavx2', '-mfma', '-march=native', '-fopenmp',
        'test_solution.cpp', 'MySolution.cpp', '-o', 'test_tune.exe'
    ], capture_output=True, text=True)
    
    if result.returncode != 0:
        print(f"Compilation error:\n{result.stderr}")
        return False
    return True

def run_test(dataset_path):
    """Run test and extract metrics"""
    try:
        result = subprocess.run(
            ['test_tune.exe', dataset_path],
            capture_output=True, text=True, timeout=1800  # 30min timeout
        )
        
        output = result.stdout
        
        # Extract metrics using regex
        build_time = re.search(r'Build time: (\d+) ms', output)
        search_time = re.search(r'Total search time: (\d+) ms', output)
        avg_search = re.search(r'Average search time: ([\d.]+) ms', output)
        dist_comps = re.search(r'Average distance computations per query: ([\d.]+)', output)
        recall1 = re.search(r'Recall@1:\s+([\d.]+)', output)
        recall10 = re.search(r'Recall@10:\s+([\d.]+)', output)
        
        if not all([build_time, search_time, recall10]):
            print(f"Failed to parse output. Missing metrics.")
            return None
        
        return {
            'build_ms': int(build_time.group(1)),
            'build_min': float(build_time.group(1)) / 60000,
            'search_ms': int(search_time.group(1)),
            'avg_search_ms': float(avg_search.group(1)) if avg_search else 0,
            'dist_comps': float(dist_comps.group(1)) if dist_comps else 0,
            'recall1': float(recall1.group(1)) if recall1 else 0,
            'recall10': float(recall10.group(1)),
        }
    
    except subprocess.TimeoutExpired:
        print("Test timed out (>30min)")
        return None
    except Exception as e:
        print(f"Test error: {e}")
        return None

def evaluate_config(M, ef_c, ef_s, dataset_path):
    """Test a single configuration"""
    print(f"\n{'='*60}")
    print(f"Testing: M={M}, ef_c={ef_c}, ef_s={ef_s}")
    print('='*60)
    
    # Modify parameters
    if not modify_params(M, ef_c, ef_s):
        return None
    
    # Compile
    print("Compiling...", end=' ', flush=True)
    if not compile_code():
        print("FAILED")
        return None
    print("OK")
    
    # Run test
    print("Running test (15-25min expected)...", flush=True)
    metrics = run_test(dataset_path)
    
    if metrics is None:
        return None
    
    # Calculate score (lower is better)
    recall_penalty = max(0, 0.98 - metrics['recall10']) * 10000  # Heavy penalty for <98%
    time_penalty = max(0, metrics['build_ms'] - 1800000) / 1000  # Penalty for >30min build
    
    # Score = build_time + search_time + penalties
    score = metrics['build_min'] + metrics['search_ms']/1000 + recall_penalty + time_penalty
    
    # Print results
    print(f"\nResults:")
    print(f"  Build time:    {metrics['build_min']:.2f} min")
    print(f"  Search time:   {metrics['search_ms']} ms ({metrics['avg_search_ms']:.2f} ms/query)")
    print(f"  Dist comps:    {metrics['dist_comps']:.0f} per query")
    print(f"  Recall@1:      {metrics['recall1']:.4f} ({metrics['recall1']*100:.2f}%)")
    print(f"  Recall@10:     {metrics['recall10']:.4f} ({metrics['recall10']*100:.2f}%)")
    
    # Status
    recall_ok = metrics['recall10'] >= 0.98
    build_ok = metrics['build_min'] <= 30
    search_ok = metrics['search_ms'] <= 2000  # Target <2s for 100 queries
    
    status_parts = []
    if recall_ok: status_parts.append("✓ Recall")
    else: status_parts.append(f"✗ Recall {metrics['recall10']*100:.1f}%")
    
    if build_ok: status_parts.append("✓ Build")
    else: status_parts.append(f"✗ Build {metrics['build_min']:.1f}min")
    
    if search_ok: status_parts.append("✓ Search")
    else: status_parts.append(f"✗ Search {metrics['search_ms']}ms")
    
    print(f"  Status: {' | '.join(status_parts)}")
    print(f"  Score: {score:.2f} (lower is better)")
    
    return {
        'M': M,
        'ef_c': ef_c,
        'ef_s': ef_s,
        'metrics': metrics,
        'score': score,
        'pass': recall_ok and build_ok
    }

def main():
    if len(sys.argv) < 2:
        print("Usage: python fine_tune.py <dataset_path> [--quick]")
        print("Example: python fine_tune.py ../data_o/data_o/glove")
        sys.exit(1)
    
    dataset_path = sys.argv[1]
    quick_mode = '--quick' in sys.argv
    
    grid = QUICK_GRID if quick_mode else PARAM_GRID
    
    print("="*60)
    print("Fine-Grained HNSW Parameter Tuning")
    print("="*60)
    print(f"Dataset: {dataset_path}")
    print(f"Mode: {'QUICK' if quick_mode else 'FULL'}")
    print(f"Parameter grid:")
    print(f"  M: {grid['M']}")
    print(f"  ef_construction: {grid['ef_construction']}")
    print(f"  ef_search: {grid['ef_search']}")
    
    # Generate all combinations
    configs = []
    for M in grid['M']:
        for ef_c in grid['ef_construction']:
            for ef_s in grid['ef_search']:
                configs.append((M, ef_c, ef_s))
    
    print(f"\nTotal configurations to test: {len(configs)}")
    est_time = len(configs) * 20  # ~20min per config
    print(f"Estimated time: {est_time/60:.1f} hours")
    
    response = input("\nContinue? (y/n): ")
    if response.lower() != 'y':
        print("Aborted")
        sys.exit(0)
    
    # Test all configurations
    results = []
    start_time = datetime.now()
    
    for i, (M, ef_c, ef_s) in enumerate(configs, 1):
        print(f"\n[{i}/{len(configs)}] Progress: {i/len(configs)*100:.1f}%")
        result = evaluate_config(M, ef_c, ef_s, dataset_path)
        
        if result:
            results.append(result)
            
            # Save intermediate results
            with open('tuning_results.json', 'w') as f:
                json.dump({
                    'timestamp': datetime.now().isoformat(),
                    'dataset': dataset_path,
                    'results': results
                }, f, indent=2)
    
    # Final summary
    end_time = datetime.now()
    elapsed = (end_time - start_time).total_seconds() / 3600
    
    print("\n" + "="*60)
    print("TUNING COMPLETE")
    print("="*60)
    print(f"Total time: {elapsed:.2f} hours")
    print(f"Configs tested: {len(results)}/{len(configs)}")
    
    if not results:
        print("\n✗ No successful configurations")
        sys.exit(1)
    
    # Filter passing configs
    passing = [r for r in results if r['pass']]
    
    if passing:
        print(f"\n✓ {len(passing)} configuration(s) passed requirements:\n")
        
        # Sort by score (best first)
        passing.sort(key=lambda x: x['score'])
        
        print(f"{'Rank':<6}{'M':<5}{'ef_c':<8}{'ef_s':<8}{'Build(m)':<10}{'Search(ms)':<12}{'Recall%':<10}{'Score':<10}")
        print("-"*75)
        
        for rank, r in enumerate(passing[:10], 1):  # Top 10
            m = r['metrics']
            print(f"{rank:<6}{r['M']:<5}{r['ef_c']:<8}{r['ef_s']:<8}"
                  f"{m['build_min']:<10.1f}{m['search_ms']:<12}{m['recall10']*100:<10.2f}"
                  f"{r['score']:<10.2f}")
        
        # Recommend best
        best = passing[0]
        print(f"\n{'='*60}")
        print("RECOMMENDED CONFIGURATION:")
        print(f"{'='*60}")
        print(f"  M = {best['M']}")
        print(f"  ef_construction = {best['ef_c']}")
        print(f"  ef_search = {best['ef_s']}")
        print(f"\nPerformance:")
        print(f"  Build time: {best['metrics']['build_min']:.2f} min")
        print(f"  Search time: {best['metrics']['search_ms']} ms")
        print(f"  Recall@10: {best['metrics']['recall10']*100:.2f}%")
        print(f"  Score: {best['score']:.2f}")
        
    else:
        print("\n✗ No configuration met all requirements")
        print("\nTop 5 by score (even if failed):")
        results.sort(key=lambda x: x['score'])
        
        for i, r in enumerate(results[:5], 1):
            m = r['metrics']
            print(f"\n{i}. M={r['M']}, ef_c={r['ef_c']}, ef_s={r['ef_s']}")
            print(f"   Build: {m['build_min']:.1f}min, Search: {m['search_ms']}ms, "
                  f"Recall: {m['recall10']*100:.2f}%, Score: {r['score']:.2f}")
    
    print(f"\nResults saved to: tuning_results.json")

if __name__ == '__main__':
    main()
