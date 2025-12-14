#!/usr/bin/env python3
"""
Quick Plot for Traffic Analysis System
Usage: python quick_plot.py <scenario_name>
"""

import pandas as pd
import matplotlib.pyplot as plt
import sys
import os
import numpy as np

def plot_traffic_data(scenario_name):
    """Plot traffic analysis results"""
    
    # File paths
    csv_file = f"output/{scenario_name}.csv"
    plot_file = f"output/{scenario_name}_plot.png"
    
    # Check if file exists
    if not os.path.exists(csv_file):
        print(f"Error: File {csv_file} not found!")
        return False
    
    # Read data
    try:
        df = pd.read_csv(csv_file)
    except Exception as e:
        print(f"Error reading CSV file: {e}")
        return False
    
    # Calculate basic statistics
    q_max = df['q'].max()
    k_opt = df.loc[df['q'].idxmax(), 'k']
    v_free = df['v'].iloc[0] if not df.empty else 0
    k_jam = df.loc[df['v'] <= 0.1, 'k'].min() if not df[df['v'] <= 0.1].empty else df['k'].max()
    
    # Create figure with subplots
    fig = plt.figure(figsize=(12, 5))
    
    # Plot 1: Speed-Density
    ax1 = plt.subplot(1, 2, 1)
    ax1.plot(df['k'], df['v'], 'b-', linewidth=2, label='Speed')
    ax1.set_xlabel('Density (veh/km)', fontsize=12)
    ax1.set_ylabel('Speed (km/h)', fontsize=12)
    ax1.set_title('Speed-Density Relationship', fontsize=14, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    
    # Plot 2: Flow-Density
    ax2 = plt.subplot(1, 2, 2)
    ax2.plot(df['k'], df['q'], 'r-', linewidth=2, label='Flow')
    ax2.plot(k_opt, q_max, 'g*', markersize=15, label=f'Capacity: {q_max:.0f} veh/h')
    ax2.set_xlabel('Density (veh/km)', fontsize=12)
    ax2.set_ylabel('Flow (veh/h)', fontsize=12)
    ax2.set_title('Flow-Density Relationship', fontsize=14, fontweight='bold')
    ax2.grid(True, alpha=0.3)
    ax2.legend()
    
    # Add summary text
    summary_text = f"""Analysis Summary:
    
Scenario: {scenario_name}
Free-flow speed: {v_free:.1f} km/h
Jam density: {k_jam:.1f} veh/km
Maximum flow: {q_max:.0f} veh/h
Optimal density: {k_opt:.1f} veh/km
Data points: {len(df)}"""
    
    plt.figtext(0.02, 0.02, summary_text, fontsize=10,
                bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.8))
    
    # Adjust layout and save
    plt.tight_layout()
    plt.savefig(plot_file, dpi=150, bbox_inches='tight')
    
    # Display plot
    plt.show()
    
    print(f"✓ Plot generated: {plot_file}")
    print(f"✓ Data points: {len(df)}")
    print(f"✓ Maximum flow: {q_max:.0f} veh/h at density {k_opt:.1f} veh/km")
    
    return True

def main():
    """Main function"""
    
    # Check command line arguments
    if len(sys.argv) != 2:
        print("Usage: python quick_plot.py <scenario_name>")
        print("Example: python quick_plot.py my_scenario")
        return 1
    
    scenario_name = sys.argv[1]
    
    # Create output directory if it doesn't exist
    os.makedirs("output", exist_ok=True)
    
    # Generate plot
    success = plot_traffic_data(scenario_name)
    
    return 0 if success else 1

if __name__ == "__main__":
    try:
        exit_code = main()
        sys.exit(exit_code)
    except KeyboardInterrupt:
        print("\nPlotting interrupted by user.")
        sys.exit(0)
    except Exception as e:
        print(f"\nError: {e}")
        sys.exit(1)