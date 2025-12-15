#!/usr/bin/env python3
"""
Fast parameter tuning - test only the most promising configurations
Based on prior knowledge, test 4 carefully selected configs
"""

import subprocess
import re
import json
from datetime import datetime

# Carefully selected configurations based on theory and experience
CONFIGS = [
    # (M, ef_c, ef_s, reason)
    (18, 150, 2400, "Current optimized - expected 98%+, 22-24min build"),
    (18, 160, 2500, "Higher quality - expected 98.5%+, 24-26min build"),
    (16, 150, 2400, "Faster build - expected 98%, 20-22min build"),
    (20, 150, 2400, "Best connectivity - expected 98.5%+, 25-27min build"),
]

def modify_params(M, ef_c, ef_s):
    """Modify MySolution.cpp parameters"""
    with open('MySolution.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    modified = False
    for i, line in enumerate(lines):
        if 'GLOVE: Fine-tuned' in line or 'GLOVE: Test config' in line:
            lines[i] = f'        // GLOVE: Test config M={M}, ef_c={ef_c}, ef_s={ef_s}\n'
            for j in range(i+1, min(i+10, len(lines))):
                if lines[j].strip().startswith('M ='):
                    lines[j] = f'        M = {M};\n'
                elif lines[j].strip().startswith('ef_construction ='):
                    lines[j] = f'        ef_construction = {ef_c};\n'
                elif lines[j].strip().startswith('ef_search ='):
                    lines[j] = f'        ef_search = {ef_s};\n'
            modified = True
            break
    
    if modified:
        with open('MySolution.cpp', 'w', encoding='utf-8') as f:
            f.writelines(lines)
    return modified

def compile_code():
    """Compile with optimizations"""
    result = subprocess.run([
        'g++', '-std=c++11', '-O3', '-mavx2', '-mfma', '-march=native', '-fopenmp',
        'test_solution.cpp', 'MySolution.cpp', '-o', 'test_quick.exe'
    ], capture_output=True, text=True)
    return result.returncode == 0

def run_test(dataset_path):
    """Run test and extract metrics"""
    try:
        result = subprocess.run(
            ['test_quick.exe', dataset_path],
            capture_output=True, text=True, timeout=1800
        )
        
        output = result.stdout
        
        build_time = re.search(r'Build time: (\d+) ms', output)
        search_time = re.search(r'Total search time: (\d+) ms', output)
        dist_comps = re.search(r'Average distance computations per query: ([\d.]+)', output)
        recall10 = re.search(r'Recall@10:\s+([\d.]+)', output)
        
        if not all([build_time, search_time, recall10]):
            return None
        
        return {
            'build_ms': int(build_time.group(1)),
            'build_min': float(build_time.group(1)) / 60000,
            'search_ms': int(search_time.group(1)),
            'dist_comps': float(dist_comps.group(1)) if dist_comps else 0,
            'recall10': float(recall10.group(1)),
        }
    
    except subprocess.TimeoutExpired:
        return None
    except Exception as e:
        print(f"Error: {e}")
        return None

def main():
    dataset_path = r'..\data_o\data_o\glove'
    
    print("="*60)
    print("FAST Parameter Tuning (4 Selected Configs)")
    print("="*60)
    print(f"Dataset: {dataset_path}")
    print(f"\nTesting {len(CONFIGS)} carefully selected configurations")
    print(f"Estimated time: ~{len(CONFIGS)*20} minutes ({len(CONFIGS)*20/60:.1f} hours)")
    print()
    
    for i, (M, ef_c, ef_s, reason) in enumerate(CONFIGS, 1):
        print(f"  {i}. M={M}, ef_c={ef_c}, ef_s={ef_s}")
        print(f"     {reason}")
    
    print()
    
    results = []
    start_time = datetime.now()
    
    for i, (M, ef_c, ef_s, reason) in enumerate(CONFIGS, 1):
        print(f"\n{'='*60}")
        print(f"[{i}/{len(CONFIGS)}] Testing: M={M}, ef_c={ef_c}, ef_s={ef_s}")
        print(f"Expected: {reason}")
        print('='*60)
        
        # Modify
        if not modify_params(M, ef_c, ef_s):
            print("✗ Failed to modify parameters")
            continue
        
        # Compile
        print("Compiling...", end=' ', flush=True)
        if not compile_code():
            print("FAILED")
            continue
        print("OK")
        
        # Run test
        print("Running test (expect 15-25 minutes)...", flush=True)
        metrics = run_test(dataset_path)
        
        if metrics is None:
            print("✗ Test failed or timed out")
            continue
        
        # Results
        print(f"\nResults:")
        print(f"  Build time:    {metrics['build_min']:.2f} min")
        print(f"  Search time:   {metrics['search_ms']} ms")
        print(f"  Dist comps:    {metrics['dist_comps']:.0f} per query")
        print(f"  Recall@10:     {metrics['recall10']:.4f} ({metrics['recall10']*100:.2f}%)")
        
        recall_ok = metrics['recall10'] >= 0.98
        build_ok = metrics['build_min'] <= 30
        search_ok = metrics['search_ms'] <= 2000
        
        status = []
        if recall_ok: status.append("✓ Recall")
        else: status.append(f"✗ Recall {metrics['recall10']*100:.1f}%")
        
        if build_ok: status.append("✓ Build")
        else: status.append(f"✗ Build {metrics['build_min']:.1f}min")
        
        if search_ok: status.append("✓ Search")
        else: status.append(f"✗ Search {metrics['search_ms']}ms")
        
        print(f"  Status: {' | '.join(status)}")
        
        # Score (lower is better)
        recall_penalty = max(0, 0.98 - metrics['recall10']) * 10000
        time_penalty = max(0, metrics['build_ms'] - 1800000) / 1000
        score = metrics['build_min'] + metrics['search_ms']/1000 + recall_penalty + time_penalty
        
        results.append({
            'M': M, 'ef_c': ef_c, 'ef_s': ef_s,
            'reason': reason,
            'metrics': metrics,
            'score': score,
            'pass': recall_ok and build_ok
        })
        
        # Save progress
        with open('fast_tuning_results.json', 'w') as f:
            json.dump({
                'timestamp': datetime.now().isoformat(),
                'results': results
            }, f, indent=2)
        
        print(f"  Score: {score:.2f}")
    
    # Summary
    end_time = datetime.now()
    elapsed = (end_time - start_time).total_seconds() / 60
    
    print(f"\n{'='*60}")
    print("SUMMARY")
    print('='*60)
    print(f"Total time: {elapsed:.1f} minutes")
    print(f"Configs tested: {len(results)}/{len(CONFIGS)}")
    
    if not results:
        print("\n✗ No successful tests")
        return
    
    # Sort by score
    results.sort(key=lambda x: x['score'])
    
    print(f"\n{'Rank':<6}{'Config':<20}{'Build(m)':<11}{'Search(ms)':<12}{'Recall%':<10}{'Score':<10}")
    print('-'*75)
    
    for rank, r in enumerate(results, 1):
        m = r['metrics']
        config_str = f"M={r['M']},ef_c={r['ef_c']},ef_s={r['ef_s']}"
        marker = "✓" if r['pass'] else "✗"
        print(f"{marker} {rank:<4}{config_str:<19}{m['build_min']:<11.1f}{m['search_ms']:<12}"
              f"{m['recall10']*100:<10.2f}{r['score']:<10.2f}")
    
    # Best config
    best = results[0]
    print(f"\n{'='*60}")
    print("RECOMMENDED CONFIGURATION:")
    print('='*60)
    print(f"  M = {best['M']}")
    print(f"  ef_construction = {best['ef_c']}")
    print(f"  ef_search = {best['ef_s']}")
    print(f"\nPerformance:")
    print(f"  Build time:  {best['metrics']['build_min']:.2f} min")
    print(f"  Search time: {best['metrics']['search_ms']} ms")
    print(f"  Recall@10:   {best['metrics']['recall10']*100:.2f}%")
    print(f"  Score:       {best['score']:.2f} (lower is better)")
    print(f"\nReason: {best['reason']}")
    
    if best['pass']:
        print(f"\n✓ This configuration meets all requirements!")
    else:
        print(f"\n⚠ Best available but may not meet all requirements")

if __name__ == '__main__':
    main()
