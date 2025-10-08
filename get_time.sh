#!/usr/bin/env bash

# ==============================================================================
# A robust script to run the msccl simulation with different chunk sizes,
# record the time taken, and generate a plot.
# ==============================================================================

set -e
set -u
set -o pipefail

# --- Global Constants ---
readonly SCRIPT_NAME="$(basename "$0")"
readonly MAX_UINT32_CHUNK_SIZE_KB=4194304
readonly GNUPLOT_SCRIPT="chunk_size_2_time.gp"
readonly RESULTS_DIR="sim-results"
readonly START_CHUNK_SIZE_KB=8  # Starting chunk size in KB

# --- Default Configuration ---
DEFAULT_CONFIG_FILE="examples/allstack/config.sh"
DEFAULT_OUTPUT_PREFIX="result"
DEFAULT_NS3_CMD="./ns3 run 'scratch/msccl/main'"

# --- Function Definitions ---

# Print usage information and exit.
usage() {
    cat <<EOF >&2
Usage: $SCRIPT_NAME -l <limit_kb> [-c <config_file>] [-o <output_prefix>] [-h]

A script to run the msccl simulation with different chunk sizes.

OPTIONS:
    -l <limit_kb>       Required. The upper limit for chunk size in KB (e.g., 65536).
    -c <config_file>   Path to the ns3 config file.
                        (Default: $DEFAULT_CONFIG_FILE)
    -o <output_prefix> Prefix for the output results.txt and plot.pdf files.
                        (Default: $DEFAULT_OUTPUT_PREFIX)
    -h                  Show this help message.
EOF
    exit 1
}

# Cleanup function to remove temporary files on exit.
# shellcheck disable=SC2154
cleanup() {
    # Check if TMP_RESULTS_FILE is set before trying to remove it
    if [ -n "${TMP_RESULTS_FILE:-}" ]; then
      rm -f "$TMP_RESULTS_FILE"
    fi
}

# Main simulation loop.
run_simulation() {
    local -r chunk_size_limit_kb=$1
    local -r config_file=$2
    local -r ns3_cmd_base=$3
    # Initialize TMP_RESULTS_FILE here to avoid unbound variable error in cleanup if mktemp fails
    TMP_RESULTS_FILE=""
    TMP_RESULTS_FILE=$(mktemp)

    # Store the original chunk size line and set a trap to restore it.
    local original_chunk_size_line
    original_chunk_size_line=$(grep "ns3::ThreadBlock::chunkSize" "$config_file") || true
    if [ -z "$original_chunk_size_line" ]; then
        echo "Error: Could not find 'ns3::ThreadBlock::chunkSize' in $config_file." >&2
        return 1 # Use return instead of exit for function
    fi
    trap -- 'echo "Restoring original chunk size in '"$config_file"'..." >&2; sed -i "s/^ns3::ThreadBlock::chunkSize .*/'"$original_chunk_size_line"'/" '"$config_file"'' RETURN
    
    local -a chunk_sizes=()
    local size=$START_CHUNK_SIZE_KB
    while [ "$size" -le "$chunk_size_limit_kb" ]; do
        chunk_sizes+=("$size")
        size=$((size * 2))
    done

    local -r ns3_run_cmd="$ns3_cmd_base -- $config_file"

    echo "Chunk Size (KB),Time Taken (ns)" > "$TMP_RESULTS_FILE"

    for chunk_size in "${chunk_sizes[@]}"; do
        echo "--> Running simulation with chunk size: ${chunk_size} KB" >&2
        
        local new_value_bytes=$((chunk_size * 1000))
        sed -i "s/ns3::ThreadBlock::chunkSize [0-9]\+/ns3::ThreadBlock::chunkSize $new_value_bytes/" "$config_file"

        echo "    Executing: $ns3_run_cmd" >&2
        local time_taken
        time_taken=$(NS_LOG="" $ns3_run_cmd | awk '/GPU rank/ {sum += $(NF-1); count++} END {if (count > 0) print sum / count; else print 0}')

        if ! [[ "$time_taken" =~ ^[0-9.eE+-]+$ ]]; then
            echo "Error: Failed to extract a valid time from ns3 output for chunk size ${chunk_size}KB." >&2
            echo "Received: '$time_taken'" >&2
            exit 1
        fi
        
        echo "$chunk_size,$time_taken" >> "$TMP_RESULTS_FILE"
        echo "    Time taken: ${time_taken} ns" >&2
    done

    echo "Simulation runs completed." >&2
}

# Generate a plot using gnuplot.
# OPTIMIZED: This function is now self-contained and only needs the data file.
generate_plot() {
    local -r results_file=$1
    local -r output_plot_path=$2

    if ! command -v gnuplot &> /dev/null; then
        echo "Warning: Gnuplot not found, skipping plot generation." >&2
        return
    fi
    
    # The logic is now entirely within the Gnuplot script.
    gnuplot -e "input_files='$results_file'" -e "output_file='$output_plot_path'" -e "title='Time of MSCCL in Simulation'" -p $GNUPLOT_SCRIPT

    echo "Plot saved to $output_plot_path" >&2
}

# --- Main Execution Logic ---
main() {
    trap cleanup EXIT INT TERM

    local chunk_size_limit_kb=""
    local config_file="$DEFAULT_CONFIG_FILE"
    local output_prefix="$DEFAULT_OUTPUT_PREFIX"

    while getopts ":l:c:o:h" opt; do
        case $opt in
            l) chunk_size_limit_kb="$OPTARG" ;;
            c) config_file="$OPTARG" ;;
            o) output_prefix="$OPTARG" ;;
            h) usage ;;
            \?) echo "Invalid option: -$OPTARG" >&2; usage ;;
            :) echo "Option -$OPTARG requires an argument." >&2; usage ;;
        esac
    done

    if [ -z "$chunk_size_limit_kb" ]; then
        echo "Error: Chunk size limit (-l) is a required argument." >&2
        usage
    fi

    if ! [[ "$chunk_size_limit_kb" =~ ^[1-9][0-9]*$ ]]; then
        echo "Error: Chunk size limit must be a positive integer." >&2
        exit 1
    fi

    if [ "$chunk_size_limit_kb" -gt "$MAX_UINT32_CHUNK_SIZE_KB" ]; then
        echo "Warning: User-defined chunk size limit ($chunk_size_limit_kb KB) exceeds plausible maximum ($MAX_UINT32_CHUNK_SIZE_KB KB)." >&2
        echo "Continuing, but please verify this is intended." >&2
    fi
    
    if [ ! -f "$config_file" ]; then
        echo "Error: Config file not found: $config_file" >&2
        exit 1
    fi

    # Create the results directory if it doesn't exist
    mkdir -p "$RESULTS_DIR"

    local -r output_file="${RESULTS_DIR}/${output_prefix}_results.txt"
    local -r output_plot="${RESULTS_DIR}/${output_prefix}_plot.pdf"

    rm -f "$output_file" "$output_plot"

    echo "Starting MSCCL simulation sweep..." >&2
    echo " - Chunk Size Limit: ${chunk_size_limit_kb} KB" >&2
    echo " - Config File:      $config_file" >&2
    echo " - Output Prefix:    $output_prefix" >&2
    
    run_simulation "$chunk_size_limit_kb" "$config_file" "$DEFAULT_NS3_CMD"
    
    cp "$TMP_RESULTS_FILE" "$output_file"
    echo "Results saved to $output_file" >&2

    generate_plot "$output_file" "$output_plot"

    echo "Script finished successfully." >&2
}

main "$@"