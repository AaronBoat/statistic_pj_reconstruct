#!/usr/bin/env python3
"""
Visualize SIFT Grid Search Results
Analyze and display results from grid search CSV using only built-in libraries
"""

import csv
import sys
from collections import defaultdict

def load_results(filename='sift_grid_search_results.csv'):
    """Load grid search results from CSV"""
    try:
        results = []
        with open(filename, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                # Convert numeric fields
                results.append({
                    'M': int(row['M']),
                    'ef_construction': int(row['ef_construction']),
                    'ef_search': int(row['ef_search']),
                    'build_time_ms': float(row['build_time_ms']),
                    'avg_search_ms': float(row['avg_search_ms']),
                    'recall_10': float(row['recall_10']),
                    'score': float(row['score'])
                })
        print(f"Loaded {len(results)} configurations from {filename}")
        return results
    except FileNotFoundError:
        print(f"Error: {filename} not found!")
        print("Please run grid_search_sift.exe first")
        return None
    except Exception as e:
        print(f"Error loading file: {e}")
        return None

def analyze_parameter_impact(results):
    """Analyze how each parameter affects performance"""
    print("\n" + "="*70)
    print("Parameter Impact Analysis")
    print("="*70)
    
    # Group by each parameter
    by_M = defaultdict(list)
    by_ef_c = defaultdict(list)
    by_ef_s = defaultdict(list)
    
    for r in results:
        by_M[r['M']].append(r)
        by_ef_c[r['ef_construction']].append(r)
        by_ef_s[r['ef_search']].append(r)
    
    # Analyze M impact
    print("\nImpact of M (averaged over other parameters):")
    print("-" * 70)
    print(f"{'M':>5} | {'Recall@10':>10} | {'Query(ms)':>10} | {'Build(s)':>10} | {'Count':>6}")
    print("-" * 70)
    for M in sorted(by_M.keys()):
        configs = by_M[M]
        avg_recall = sum(c['recall_10'] for c in configs) / len(configs)
        avg_query = sum(c['avg_search_ms'] for c in configs) / len(configs)
        avg_build = sum(c['build_time_ms'] for c in configs) / len(configs) / 1000
        print(f"{M:>5} | {avg_recall:>10.4f} | {avg_query:>10.2f} | {avg_build:>10.1f} | {len(configs):>6}")
    
    # Analyze ef_construction impact
    print("\nImpact of ef_construction (averaged over other parameters):")
    print("-" * 70)
    print(f"{'ef_c':>5} | {'Recall@10':>10} | {'Query(ms)':>10} | {'Build(s)':>10} | {'Count':>6}")
    print("-" * 70)
    for ef_c in sorted(by_ef_c.keys()):
        configs = by_ef_c[ef_c]
        avg_recall = sum(c['recall_10'] for c in configs) / len(configs)
        avg_query = sum(c['avg_search_ms'] for c in configs) / len(configs)
        avg_build = sum(c['build_time_ms'] for c in configs) / len(configs) / 1000
        print(f"{ef_c:>5} | {avg_recall:>10.4f} | {avg_query:>10.2f} | {avg_build:>10.1f} | {len(configs):>6}")
    
    # Analyze ef_search impact
    print("\nImpact of ef_search (averaged over other parameters):")
    print("-" * 70)
    print(f"{'ef_s':>5} | {'Recall@10':>10} | {'Query(ms)':>10} | {'Build(s)':>10} | {'Count':>6}")
    print("-" * 70)
    for ef_s in sorted(by_ef_s.keys()):
        configs = by_ef_s[ef_s]
        avg_recall = sum(c['recall_10'] for c in configs) / len(configs)
        avg_query = sum(c['avg_search_ms'] for c in configs) / len(configs)
        avg_build = sum(c['build_time_ms'] for c in configs) / len(configs) / 1000
        print(f"{ef_s:>5} | {avg_recall:>10.4f} | {avg_query:>10.2f} | {avg_build:>10.1f} | {len(configs):>6}")

def find_pareto_optimal(results):
    """Find Pareto optimal configurations (recall vs speed trade-off)"""
    print("\n" + "="*70)
    print("Pareto Optimal Configurations (Recall vs Query Speed)")
    print("="*70)
    
    # Sort by recall descending
    sorted_by_recall = sorted(results, key=lambda x: x['recall_10'], reverse=True)
    
    pareto = []
    best_speed = float('inf')
    
    for config in sorted_by_recall:
        if config['avg_search_ms'] < best_speed:
            pareto.append(config)
            best_speed = config['avg_search_ms']
    
    print(f"\nFound {len(pareto)} Pareto optimal configurations:")
    print("-" * 70)
    print(f"{'M':>3} | {'ef_c':>5} | {'ef_s':>5} | {'Recall@10':>10} | {'Query(ms)':>10} | {'Score':>8}")
    print("-" * 70)
    
    for config in pareto:
        print(f"{config['M']:>3} | {config['ef_construction']:>5} | {config['ef_search']:>5} | "
              f"{config['recall_10']:>10.4f} | {config['avg_search_ms']:>10.2f} | {config['score']:>8.2f}")

def generate_html_report(results, filename='sift_grid_search_report.html'):
    """Generate an HTML report with interactive table"""
    html = """<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>SIFT Grid Search Results</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        h1 { color: #333; }
        table { border-collapse: collapse; width: 100%; background: white; margin: 20px 0; }
        th, td { border: 1px solid #ddd; padding: 10px; text-align: center; }
        th { background-color: #4CAF50; color: white; cursor: pointer; }
        tr:nth-child(even) { background-color: #f2f2f2; }
        tr:hover { background-color: #ddd; }
        .high-recall { background-color: #c8e6c9 !important; }
        .fast-query { background-color: #bbdefb !important; }
        .summary { background: white; padding: 20px; margin: 20px 0; border-radius: 5px; }
    </style>
    <script>
        function sortTable(n) {
            var table = document.getElementById("resultsTable");
            var rows = Array.from(table.rows).slice(1);
            var dir = table.rows[0].cells[n].getAttribute('data-dir') || 'asc';
            
            rows.sort((a, b) => {
                var x = parseFloat(a.cells[n].textContent);
                var y = parseFloat(b.cells[n].textContent);
                return dir === 'asc' ? x - y : y - x;
            });
            
            dir = dir === 'asc' ? 'desc' : 'asc';
            table.rows[0].cells[n].setAttribute('data-dir', dir);
            
            rows.forEach(row => table.appendChild(row));
        }
    </script>
</head>
<body>
    <h1>SIFT Grid Search Results</h1>
"""
    
    # Add summary statistics
    html += '<div class="summary">\n'
    html += f'<h2>Summary</h2>\n'
    html += f'<p><strong>Total configurations:</strong> {len(results)}</p>\n'
    
    best_recall = max(results, key=lambda x: x['recall_10'])
    html += f'<p><strong>Best Recall@10:</strong> {best_recall["recall_10"]:.4f} '
    html += f'(M={best_recall["M"]}, ef_c={best_recall["ef_construction"]}, ef_s={best_recall["ef_search"]})</p>\n'
    
    best_speed = min(results, key=lambda x: x['avg_search_ms'])
    html += f'<p><strong>Best Query Speed:</strong> {best_speed["avg_search_ms"]:.2f}ms '
    html += f'(M={best_speed["M"]}, ef_c={best_speed["ef_construction"]}, ef_s={best_speed["ef_search"]})</p>\n'
    
    best_score = max(results, key=lambda x: x['score'])
    html += f'<p><strong>Best Overall Score:</strong> {best_score["score"]:.2f} '
    html += f'(M={best_score["M"]}, ef_c={best_score["ef_construction"]}, ef_s={best_score["ef_search"]})</p>\n'
    html += '</div>\n'
    
    # Add table
    html += '<table id="resultsTable">\n<tr>\n'
    headers = ['M', 'ef_construction', 'ef_search', 'Build Time (s)', 'Query Time (ms)', 'Recall@10', 'Score']
    for i, header in enumerate(headers):
        html += f'<th onclick="sortTable({i})">{header}</th>\n'
    html += '</tr>\n'
    
    for r in sorted(results, key=lambda x: x['score'], reverse=True):
        row_class = ''
        if r['recall_10'] >= 0.98:
            row_class = ' class="high-recall"'
        elif r['avg_search_ms'] <= 1.0:
            row_class = ' class="fast-query"'
            
        html += f'<tr{row_class}>\n'
        html += f'<td>{r["M"]}</td>\n'
        html += f'<td>{r["ef_construction"]}</td>\n'
        html += f'<td>{r["ef_search"]}</td>\n'
        html += f'<td>{r["build_time_ms"]/1000:.1f}</td>\n'
        html += f'<td>{r["avg_search_ms"]:.2f}</td>\n'
        html += f'<td>{r["recall_10"]:.4f}</td>\n'
        html += f'<td>{r["score"]:.2f}</td>\n'
        html += '</tr>\n'
    
    html += '</table>\n</body>\n</html>'
    
    with open(filename, 'w', encoding='utf-8') as f:
        f.write(html)
    
    print(f"\nGenerated HTML report: {filename}")

def print_summary(results):
    """Print summary statistics"""
    print("\n" + "="*70)
    print("Summary Statistics")
    print("="*70)
    
    recalls = [r['recall_10'] for r in results]
    query_times = [r['avg_search_ms'] for r in results]
    build_times = [r['build_time_ms']/1000 for r in results]
    
    print(f"\nTotal configurations tested: {len(results)}")
    print(f"Recall@10 range: {min(recalls):.4f} - {max(recalls):.4f}")
    print(f"Query time range: {min(query_times):.2f}ms - {max(query_times):.2f}ms")
    print(f"Build time range: {min(build_times):.1f}s - {max(build_times):.1f}s")
    
    print("\n" + "-"*70)
    print("Best Configurations:")
    print("-"*70)
    
    # Best recall
    best_recall = max(results, key=lambda x: x['recall_10'])
    print(f"\n✓ Best Recall@10: {best_recall['recall_10']:.4f}")
    print(f"  Parameters: M={best_recall['M']}, ef_construction={best_recall['ef_construction']}, "
          f"ef_search={best_recall['ef_search']}")
    print(f"  Query time: {best_recall['avg_search_ms']:.2f}ms, Build time: {best_recall['build_time_ms']/1000:.1f}s")
    
    # Best speed
    best_speed = min(results, key=lambda x: x['avg_search_ms'])
    print(f"\n✓ Best Query Speed: {best_speed['avg_search_ms']:.2f}ms")
    print(f"  Parameters: M={best_speed['M']}, ef_construction={best_speed['ef_construction']}, "
          f"ef_search={best_speed['ef_search']}")
    print(f"  Recall@10: {best_speed['recall_10']:.4f}, Build time: {best_speed['build_time_ms']/1000:.1f}s")
    
    # Best score
    best_score = max(results, key=lambda x: x['score'])
    print(f"\n✓ Best Overall Score: {best_score['score']:.2f}")
    print(f"  Parameters: M={best_score['M']}, ef_construction={best_score['ef_construction']}, "
          f"ef_search={best_score['ef_search']}")
    print(f"  Recall@10: {best_score['recall_10']:.4f}, Query time: {best_score['avg_search_ms']:.2f}ms")
    
    # Configurations with Recall >= 98%
    high_recall = [r for r in results if r['recall_10'] >= 0.98]
    if high_recall:
        print(f"\n✓ {len(high_recall)} configurations with Recall@10 >= 98%")
        best_fast = min(high_recall, key=lambda x: x['avg_search_ms'])
        print(f"  Fastest among them: M={best_fast['M']}, ef_construction={best_fast['ef_construction']}, "
              f"ef_search={best_fast['ef_search']}")
        print(f"  Query time: {best_fast['avg_search_ms']:.2f}ms, Recall@10: {best_fast['recall_10']:.4f}")
    
    # Configurations with Recall >= 95%
    decent_recall = [r for r in results if r['recall_10'] >= 0.95]
    if decent_recall:
        print(f"\n✓ {len(decent_recall)} configurations with Recall@10 >= 95%")

def print_top_configs(results, n=10):
    """Print top N configurations"""
    print("\n" + "="*70)
    print(f"Top {n} Configurations by Overall Score")
    print("="*70)
    
    top = sorted(results, key=lambda x: x['score'], reverse=True)[:n]
    
    print(f"\n{'Rank':>4} | {'M':>3} | {'ef_c':>5} | {'ef_s':>5} | {'Recall':>8} | {'Query(ms)':>10} | {'Score':>8}")
    print("-" * 70)
    
    for i, config in enumerate(top, 1):
        print(f"{i:>4} | {config['M']:>3} | {config['ef_construction']:>5} | {config['ef_search']:>5} | "
              f"{config['recall_10']:>8.4f} | {config['avg_search_ms']:>10.2f} | {config['score']:>8.2f}")

def main():
    """Main function"""
    print("\n" + "="*70)
    print("SIFT Grid Search Results Analysis")
    print("="*70)
    
    # Load results
    results = load_results()
    if results is None:
        return
    
    # Print summary
    print_summary(results)
    
    # Print top configurations
    print_top_configs(results, n=10)
    
    # Analyze parameter impact
    analyze_parameter_impact(results)
    
    # Find Pareto optimal
    find_pareto_optimal(results)
    
    # Generate HTML report
    try:
        generate_html_report(results)
        print("\n" + "="*70)
        print("Analysis complete!")
        print("="*70)
        print("\nGenerated files:")
        print("  - sift_grid_search_report.html (interactive table)")
        print("\nOpen sift_grid_search_report.html in your browser to view results.")
    except Exception as e:
        print(f"\nError generating HTML report: {e}")

if __name__ == "__main__":
    main()
