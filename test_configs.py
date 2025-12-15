#!/usr/bin/env python3
"""
Quick configuration testing script
Tests multiple M and ef_construction combinations to find fastest config with >=98% recall
"""

import subprocess
import re
import time

# Test configurations: (M, ef_construction, ef_search)
configs = [
    (12, 100, 2800),
    (12, 120, 2800),
    (14, 100, 2800),
    (14, 120, 2800),
    (16, 100, 2800),
]

def modify_config(M, ef_c, ef_s):
    """Modify MySolution.cpp with new configuration"""
    with open('MySolution.cpp', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Replace the GLOVE config section
    pattern = r'// GLOVE:.*?\n\s+M = \d+;\n\s+ef_construction = \d+;\n\s+ef_search = \d+;'
    replacement = f'''// GLOVE: Test config M={M}, ef_c={ef_c}, ef_s={ef_s}
        M = {M};
        ef_construction = {ef_c};
        ef_search = {ef_s};'''
    
    content = re.sub(pattern, replacement, content, flags=re.MULTILINE)
    
    with open('MySolution.cpp', 'w', encoding='utf-8') as f:
        f.write(content)

def compile_code():
    """Compile the code"""
    result = subprocess.run([
        'g++', '-std=c++11', '-O3', '-mavx2', '-mfma', '-march=native', '-fopenmp',
        'test_solution.cpp', 'MySolution.cpp', '-o', 'test_cfg.exe'
    ], capture_output=True, text=True)
    return result.returncode == 0

def run_test():
    """Run test and extract metrics"""
    result = subprocess.run(['test_cfg.exe', '..\\data_o\\data_o\\glove'],
                          capture_output=True, text=True, timeout=1200)
    
    output = result.stdout
    
    # Extract metrics
    build_time = re.search(r'Build time: (\d+) ms', output)
    search_time = re.search(r'Total search time: (\d+) ms', output)
    dist_comps = re.search(r'Average distance computations per query: ([\d.]+)', output)
    recall = re.search(r'Recall@10: ([\d.]+)', output)
    
    if not all([build_time, search_time, dist_comps, recall]):
        return None
    
    return {
        'build_ms': int(build_time.group(1)),
        'search_ms': int(search_time.group(1)),
        'dist_comps': float(dist_comps.group(1)),
        'recall': float(recall.group(1))
    }

def main():
    print("=" * 60)
    print("Configuration Testing for GLOVE")
    print("=" * 60)
    print(f"Testing {len(configs)} configurations...\n")
    
    results = []
    
    for i, (M, ef_c, ef_s) in enumerate(configs, 1):
        print(f"\n[{i}/{len(configs)}] Testing M={M}, ef_c={ef_c}, ef_s={ef_s}")
        print("-" * 60)
        
        # Modify config
        modify_config(M, ef_c, ef_s)
        
        # Compile
        print("  Compiling...")
        if not compile_code():
            print("  ✗ Compilation failed, skipping")
            continue
        
        # Run test
        print("  Running test (this may take 15-20 minutes)...")
        start = time.time()
        try:
            metrics = run_test()
            elapsed = time.time() - start
            
            if metrics is None:
                print(f"  ✗ Test failed after {elapsed:.0f}s")
                continue
            
            # Print results
            print(f"  ✓ Test completed in {elapsed:.0f}s")
            print(f"    Build time:    {metrics['build_ms']/1000:.1f}s ({metrics['build_ms']/60000:.1f}min)")
            print(f"    Search time:   {metrics['search_ms']}ms")
            print(f"    Dist comps:    {metrics['dist_comps']:.0f}")
            print(f"    Recall@10:     {metrics['recall']:.4f} ({metrics['recall']*100:.2f}%)")
            
            # Check if meets requirements
            recall_ok = metrics['recall'] >= 0.98
            build_ok = metrics['build_ms'] <= 1800000  # 30 minutes
            
            status = "✓ PASS" if (recall_ok and build_ok) else "✗ FAIL"
            if not recall_ok:
                status += " (recall < 98%)"
            if not build_ok:
                status += " (build > 30min)"
            print(f"    Status: {status}")
            
            results.append({
                'M': M,
                'ef_c': ef_c,
                'ef_s': ef_s,
                'metrics': metrics,
                'pass': recall_ok and build_ok
            })
            
        except subprocess.TimeoutExpired:
            print("  ✗ Test timed out (>20 min)")
        except Exception as e:
            print(f"  ✗ Error: {e}")
    
    # Summary
    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    
    passing = [r for r in results if r['pass']]
    
    if passing:
        print(f"\n✓ {len(passing)} configuration(s) passed:\n")
        
        # Sort by build time
        passing.sort(key=lambda x: x['metrics']['build_ms'])
        
        for r in passing:
            m = r['metrics']
            print(f"  M={r['M']}, ef_c={r['ef_c']}, ef_s={r['ef_s']}")
            print(f"    Build: {m['build_ms']/60000:.1f}min, Search: {m['search_ms']}ms, Recall: {m['recall']*100:.2f}%")
        
        # Recommend fastest
        best = passing[0]
        print(f"\n✓ RECOMMENDED: M={best['M']}, ef_c={best['ef_c']}, ef_s={best['ef_s']}")
        print(f"  (Fastest build time: {best['metrics']['build_ms']/60000:.1f}min)")
    else:
        print("\n✗ No configuration passed all requirements")
        print("\nAll results:")
        for r in results:
            m = r['metrics']
            print(f"  M={r['M']}, ef_c={r['ef_c']}: "
                  f"Build {m['build_ms']/60000:.1f}min, Recall {m['recall']*100:.2f}%")

if __name__ == '__main__':
    main()
