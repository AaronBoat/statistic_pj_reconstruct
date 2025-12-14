#!/usr/bin/env python3
"""
NGT Adaptive Search Parameter Tuning
测试不同gamma值对性能的影响
"""
import subprocess
import re
import os
from collections import defaultdict

# 测试配置
DATASET = r"..\data_o\data_o\glove"
GAMMA_VALUES = [0.0, 0.10, 0.15, 0.19, 0.22, 0.25, 0.30]
EF_SEARCH_VALUES = [2400, 2600, 2800, 3000]

# 固定参数
M = 20
EF_CONSTRUCTION = 165

def modify_code_gamma(gamma):
    """修改MySolution.cpp中的gamma默认值"""
    with open('MySolution.cpp', 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 查找并替换gamma初始化
    pattern = r'gamma = [0-9.]+;'
    replacement = f'gamma = {gamma:.2f};'
    new_content = re.sub(pattern, replacement, content)
    
    with open('MySolution.cpp', 'w', encoding='utf-8') as f:
        f.write(new_content)
    
    print(f"  修改gamma为 {gamma:.2f}")

def compile_solution():
    """编译测试程序"""
    cmd = ['g++', '-std=c++11', '-O3', '-mavx2', '-mfma', '-march=native', '-fopenmp',
           'test_solution.cpp', 'MySolution.cpp', '-o', 'test_gamma.exe']
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.returncode == 0

def run_test(dataset):
    """运行测试并解析结果"""
    cmd = ['test_gamma.exe', dataset]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=3600)
        output = result.stdout
        
        # 解析结果
        build_time = None
        recall = None
        search_time = None
        avg_dist = None
        
        for line in output.split('\n'):
            if 'Build time:' in line:
                match = re.search(r'(\d+)\s*ms', line)
                if match:
                    build_time = int(match.group(1))
            elif 'Recall@10:' in line:
                match = re.search(r'([0-9.]+)', line)
                if match:
                    recall = float(match.group(1))
            elif 'Total search time:' in line:
                match = re.search(r'(\d+)\s*ms', line)
                if match:
                    search_time = int(match.group(1))
            elif 'Average distance computations per query:' in line:
                match = re.search(r'([0-9.]+)', line)
                if match:
                    avg_dist = float(match.group(1))
        
        return {
            'build_time': build_time,
            'recall': recall,
            'search_time': search_time,
            'avg_dist': avg_dist
        }
    except subprocess.TimeoutExpired:
        print("  测试超时!")
        return None
    except Exception as e:
        print(f"  测试失败: {e}")
        return None

def main():
    print("=" * 60)
    print("NGT自适应搜索参数调优")
    print("=" * 60)
    print()
    
    results = []
    
    for gamma in GAMMA_VALUES:
        print(f"\n【测试 gamma={gamma:.2f}】")
        print("-" * 60)
        
        # 修改gamma值
        modify_code_gamma(gamma)
        
        # 编译
        print("  编译中...")
        if not compile_solution():
            print("  ✗ 编译失败，跳过")
            continue
        print("  ✓ 编译成功")
        
        # 运行测试
        print(f"  运行测试 (M={M}, ef_c={EF_CONSTRUCTION}, ef_s=2800)...")
        result = run_test(DATASET)
        
        if result:
            results.append({
                'gamma': gamma,
                **result
            })
            
            print(f"  ✓ 完成")
            print(f"    召回率: {result['recall']:.4f}")
            print(f"    构建时间: {result['build_time']/1000/60:.1f} 分钟")
            print(f"    搜索时间: {result['search_time']/1000:.2f} 秒")
            print(f"    平均距离计算: {result['avg_dist']:.0f} 次")
        else:
            print("  ✗ 测试失败")
    
    # 输出总结
    print("\n" + "=" * 60)
    print("测试结果汇总")
    print("=" * 60)
    print()
    print(f"{'Gamma':<8} {'召回率':<10} {'搜索时间(s)':<14} {'距离计算':<12} {'构建(min)':<12}")
    print("-" * 60)
    
    for r in results:
        print(f"{r['gamma']:<8.2f} {r['recall']:<10.4f} {r['search_time']/1000:<14.2f} "
              f"{r['avg_dist']:<12.0f} {r['build_time']/1000/60:<12.1f}")
    
    # 找出最优配置
    if results:
        print()
        # 筛选召回率>=0.98的结果
        valid_results = [r for r in results if r['recall'] >= 0.98]
        
        if valid_results:
            # 按搜索时间排序
            best_by_time = min(valid_results, key=lambda x: x['search_time'])
            # 按距离计算排序
            best_by_dist = min(valid_results, key=lambda x: x['avg_dist'])
            
            print("【推荐配置】")
            print(f"  最快搜索: gamma={best_by_time['gamma']:.2f}, "
                  f"召回={best_by_time['recall']:.4f}, "
                  f"搜索={best_by_time['search_time']/1000:.2f}s")
            print(f"  最少距离: gamma={best_by_dist['gamma']:.2f}, "
                  f"召回={best_by_dist['recall']:.4f}, "
                  f"距离={best_by_dist['avg_dist']:.0f}次")
    
    print()
    print("=" * 60)

if __name__ == '__main__':
    main()
