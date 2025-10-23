
import pandas as pd
import matplotlib.pyplot as plt
import argparse
import os

def plot_slowdown(sim_time_file, exec_time_file, title, output_file):
    """
    Generates a plot showing the slowdown factor based on simulation and execution times.

    Args:
        sim_time_file (str): Path to the file with simulation times.
        exec_time_file (str): Path to the file with execution times.
        title (str): The title for the plot.
        output_file (str): The path to save the output plot image.
    """
    # Read the data from the files
    try:
        sim_df = pd.read_csv(sim_time_file)
        exec_df = pd.read_csv(exec_time_file)
    except FileNotFoundError as e:
        print(f"Error: {e}. Please check the file paths.")
        return

    # Rename columns for clarity, assuming the second column is the time
    sim_df.columns = ['Chunk Size (KB)', 'Simulation Time (s)']
    exec_df.columns = ['Chunk Size (KB)', 'Execution Time (s)']

    # Merge the two dataframes on 'Chunk Size (KB)'
    merged_df = pd.merge(sim_df, exec_df, on='Chunk Size (KB)')

    # Calculate the slowdown, avoiding division by zero
    merged_df['Slowdown'] = merged_df.apply(
        lambda row: row['Execution Time (s)'] / row['Simulation Time (s)'] if row['Simulation Time (s)'] != 0 else 0,
        axis=1
    )

    # Create the plot
    plt.figure(figsize=(10, 6))
    plt.plot(merged_df['Chunk Size (KB)'], merged_df['Slowdown'], marker='o', linestyle='-')

    # Set plot title and labels
    plt.title(title)
    plt.xlabel('Chunk Size (KB)')
    plt.ylabel('Slowdown (Execution Time / Simulation Time)')
    
    # Use a logarithmic scale for the x-axis for better visualization
    plt.xscale('log')
    plt.grid(True, which="both", ls="--")

    # Save the plot to a file
    plt.savefig(output_file)
    print(f"Plot saved to {output_file}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot slowdown from simulation and execution time data.")
    parser.add_argument('--sim-time-file', type=str, required=True,
                        help='Path to the CSV file containing simulation times.')
    parser.add_argument('--exec-time-file', type=str, required=True,
                        help='Path to the CSV file containing execution times.')
    parser.add_argument('--title', type=str, default="Slowdown vs. Chunk Size",
                        help='Title for the plot.')
    parser.add_argument('--output-file', type=str, default="slowdown_plot.png",
                        help='Name of the output image file.')

    args = parser.parse_args()

    plot_slowdown(args.sim_time_file, args.exec_time_file, args.title, args.output_file)
