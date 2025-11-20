#!/usr/bin/env python3
"""
Visualize SIFT Grid Search Results
Generate heatmaps and charts from grid search CSV
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import sys

def load_results(filename='sift_grid_search_results.csv'):
    """Load grid search results from CSV"""
    try:
        df = pd.read_csv(filename)
        print(f"Loaded {len(df)} results from {filename}")
        return df
    except FileNotFoundError:
        print(f"Error: {filename} not found!")
        print("Please run grid_search_sift.exe first")
        return None

def plot_recall_vs_params(df):
    """Plot Recall@10 vs parameters"""
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    
    # Recall vs M
    for ef_s in sorted(df['ef_search'].unique()):
        data = df[df['ef_search'] == ef_s].groupby('M')['recall_10'].mean()
        axes[0].plot(data.index, data.values, marker='o', label=f'ef_search={ef_s}')
    axes[0].set_xlabel('M')
    axes[0].set_ylabel('Recall@10')
    axes[0].set_title('Recall@10 vs M')
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)
    
    # Recall vs ef_construction
    for ef_s in sorted(df['ef_search'].unique()):
        data = df[df['ef_search'] == ef_s].groupby('ef_construction')['recall_10'].mean()
        axes[1].plot(data.index, data.values, marker='o', label=f'ef_search={ef_s}')
    axes[1].set_xlabel('ef_construction')
    axes[1].set_ylabel('Recall@10')
    axes[1].set_title('Recall@10 vs ef_construction')
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)
    
    # Recall vs ef_search
    for M in sorted(df['M'].unique()):
        data = df[df['M'] == M].groupby('ef_search')['recall_10'].mean()
        axes[2].plot(data.index, data.values, marker='o', label=f'M={M}')
    axes[2].set_xlabel('ef_search')
    axes[2].set_ylabel('Recall@10')
    axes[2].set_title('Recall@10 vs ef_search')
    axes[2].legend()
    axes[2].grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig('sift_recall_vs_params.png', dpi=150)
    print("Saved: sift_recall_vs_params.png")
    
def plot_heatmaps(df):
    """Plot heatmaps for different parameter combinations"""
    fig, axes = plt.subplots(2, 2, figsize=(14, 12))
    
    # Heatmap 1: Recall@10 (M vs ef_construction, averaged over ef_search)
    pivot1 = df.groupby(['M', 'ef_construction'])['recall_10'].mean().unstack()
    sns.heatmap(pivot1, annot=True, fmt='.4f', cmap='RdYlGn', ax=axes[0, 0])
    axes[0, 0].set_title('Recall@10 (M vs ef_construction)')
    axes[0, 0].set_ylabel('M')
    
    # Heatmap 2: Query time (M vs ef_search, averaged over ef_construction)
    pivot2 = df.groupby(['M', 'ef_search'])['avg_search_ms'].mean().unstack()
    sns.heatmap(pivot2, annot=True, fmt='.2f', cmap='RdYlGn_r', ax=axes[0, 1])
    axes[0, 1].set_title('Query Time (ms) (M vs ef_search)')
    axes[0, 1].set_ylabel('M')
    
    # Heatmap 3: Build time (M vs ef_construction, averaged over ef_search)
    pivot3 = df.groupby(['M', 'ef_construction'])['build_time_ms'].mean().unstack() / 1000
    sns.heatmap(pivot3, annot=True, fmt='.1f', cmap='RdYlGn_r', ax=axes[1, 0])
    axes[1, 0].set_title('Build Time (s) (M vs ef_construction)')
    axes[1, 0].set_ylabel('M')
    
    # Heatmap 4: Overall score (M vs ef_search, averaged over ef_construction)
    pivot4 = df.groupby(['M', 'ef_search'])['score'].mean().unstack()
    sns.heatmap(pivot4, annot=True, fmt='.1f', cmap='RdYlGn', ax=axes[1, 1])
    axes[1, 1].set_title('Overall Score (M vs ef_search)')
    axes[1, 1].set_ylabel('M')
    
    plt.tight_layout()
    plt.savefig('sift_parameter_heatmaps.png', dpi=150)
    print("Saved: sift_parameter_heatmaps.png")

def plot_pareto_front(df):
    """Plot Pareto front: Recall vs Query Time"""
    plt.figure(figsize=(10, 6))
    
    # Color by M value
    for M in sorted(df['M'].unique()):
        subset = df[df['M'] == M]
        plt.scatter(subset['avg_search_ms'], subset['recall_10'], 
                   label=f'M={M}', alpha=0.6, s=50)
    
    plt.xlabel('Average Query Time (ms)')
    plt.ylabel('Recall@10')
    plt.title('Pareto Front: Recall@10 vs Query Time')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig('sift_pareto_front.png', dpi=150)
    print("Saved: sift_pareto_front.png")

def plot_top_configurations(df, n=10):
    """Plot top N configurations by different metrics"""
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    
    # Top by Recall
    top_recall = df.nlargest(n, 'recall_10')
    labels = [f"M={row['M']}, ef_c={row['ef_construction']}, ef_s={row['ef_search']}" 
              for _, row in top_recall.iterrows()]
    axes[0].barh(range(n), top_recall['recall_10'].values)
    axes[0].set_yticks(range(n))
    axes[0].set_yticklabels(labels, fontsize=8)
    axes[0].set_xlabel('Recall@10')
    axes[0].set_title(f'Top {n} by Recall@10')
    axes[0].invert_yaxis()
    
    # Top by Speed
    top_speed = df.nsmallest(n, 'avg_search_ms')
    labels = [f"M={row['M']}, ef_c={row['ef_construction']}, ef_s={row['ef_search']}" 
              for _, row in top_speed.iterrows()]
    axes[1].barh(range(n), top_speed['avg_search_ms'].values)
    axes[1].set_yticks(range(n))
    axes[1].set_yticklabels(labels, fontsize=8)
    axes[1].set_xlabel('Query Time (ms)')
    axes[1].set_title(f'Top {n} by Query Speed')
    axes[1].invert_yaxis()
    
    # Top by Score
    top_score = df.nlargest(n, 'score')
    labels = [f"M={row['M']}, ef_c={row['ef_construction']}, ef_s={row['ef_search']}" 
              for _, row in top_score.iterrows()]
    axes[2].barh(range(n), top_score['score'].values)
    axes[2].set_yticks(range(n))
    axes[2].set_yticklabels(labels, fontsize=8)
    axes[2].set_xlabel('Overall Score')
    axes[2].set_title(f'Top {n} by Overall Score')
    axes[2].invert_yaxis()
    
    plt.tight_layout()
    plt.savefig('sift_top_configurations.png', dpi=150)
    print("Saved: sift_top_configurations.png")

def print_summary(df):
    """Print summary statistics"""
    print("\n" + "="*60)
    print("Summary Statistics")
    print("="*60)
    
    print(f"\nTotal configurations tested: {len(df)}")
    print(f"Recall@10 range: {df['recall_10'].min():.4f} - {df['recall_10'].max():.4f}")
    print(f"Query time range: {df['avg_search_ms'].min():.2f}ms - {df['avg_search_ms'].max():.2f}ms")
    print(f"Build time range: {df['build_time_ms'].min()/1000:.1f}s - {df['build_time_ms'].max()/1000:.1f}s")
    
    print("\n" + "-"*60)
    print("Best Configurations:")
    print("-"*60)
    
    # Best recall
    best_recall = df.loc[df['recall_10'].idxmax()]
    print(f"\nBest Recall@10: {best_recall['recall_10']:.4f}")
    print(f"  M={int(best_recall['M'])}, ef_c={int(best_recall['ef_construction'])}, ef_s={int(best_recall['ef_search'])}")
    print(f"  Query time: {best_recall['avg_search_ms']:.2f}ms")
    
    # Best speed
    best_speed = df.loc[df['avg_search_ms'].idxmin()]
    print(f"\nBest Query Speed: {best_speed['avg_search_ms']:.2f}ms")
    print(f"  M={int(best_speed['M'])}, ef_c={int(best_speed['ef_construction'])}, ef_s={int(best_speed['ef_search'])}")
    print(f"  Recall@10: {best_speed['recall_10']:.4f}")
    
    # Best score
    best_score = df.loc[df['score'].idxmax()]
    print(f"\nBest Overall Score: {best_score['score']:.2f}")
    print(f"  M={int(best_score['M'])}, ef_c={int(best_score['ef_construction'])}, ef_s={int(best_score['ef_search'])}")
    print(f"  Recall@10: {best_score['recall_10']:.4f}, Query time: {best_score['avg_search_ms']:.2f}ms")
    
    # Configurations with Recall > 95%
    high_recall = df[df['recall_10'] >= 0.95]
    if len(high_recall) > 0:
        print(f"\n{len(high_recall)} configurations with Recall@10 >= 95%")
        best_fast = high_recall.loc[high_recall['avg_search_ms'].idxmin()]
        print(f"  Fastest among them: M={int(best_fast['M'])}, ef_c={int(best_fast['ef_construction'])}, ef_s={int(best_fast['ef_search'])}")
        print(f"  Query time: {best_fast['avg_search_ms']:.2f}ms, Recall@10: {best_fast['recall_10']:.4f}")

def main():
    """Main function"""
    print("SIFT Grid Search Results Visualization")
    print("="*60)
    
    # Load results
    df = load_results()
    if df is None:
        return
    
    # Print summary
    print_summary(df)
    
    # Generate plots
    print("\n" + "="*60)
    print("Generating visualizations...")
    print("="*60)
    
    try:
        plot_recall_vs_params(df)
        plot_heatmaps(df)
        plot_pareto_front(df)
        plot_top_configurations(df)
        
        print("\n" + "="*60)
        print("All visualizations generated successfully!")
        print("="*60)
        print("\nGenerated files:")
        print("  - sift_recall_vs_params.png")
        print("  - sift_parameter_heatmaps.png")
        print("  - sift_pareto_front.png")
        print("  - sift_top_configurations.png")
        
    except Exception as e:
        print(f"\nError generating plots: {e}")
        print("Make sure matplotlib and seaborn are installed:")
        print("  pip install matplotlib seaborn pandas")

if __name__ == "__main__":
    main()
