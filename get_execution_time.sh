#!/usr/bin/env bash

# ==============================================================================
# Script to measure ns3 simulation execution time vs chunk size
# (Measures actual wall-clock time, not simulation time)
# ==============================================================================

set -e
set -u
set -o pipefail

# --- Constants ---
readonly SCRIPT_NAME="$(basename "$0")"
readonly GNUPLOT_SCRIPT="chunk_size_2_time.gp"
readonly RESULTS_DIR="sim-time"
readonly START_CHUNK_SIZE_KB=1

# --- Defaults ---
DEFAULT_CONFIG_FILE="examples/allstack/config.sh"
DEFAULT_OUTPUT_PREFIX="exec_time"
DEFAULT_NS3_CMD="./ns3 run 'scratch/msccl/main'"

# --- Functions ---

usage() {
    cat <<EOF >&2
Usage: $SCRIPT_NAME -l <limit_kb> [-c <config_file>] [-o <output_prefix>] [-h]

Measure ns3 simulation execution time (wall-clock) vs chunk size.

OPTIONS:
    -l <limit_kb>       Required. Upper limit for chunk size in KB.
    -c <config_file>    Path to ns3 config file (Default: $DEFAULT_CONFIG_FILE)
    -o <output_prefix>  Output file prefix (Default: $DEFAULT_OUTPUT_PREFIX)
    -h                  Show help
EOF
    exit 1
}

cleanup() {
    if [ -n "${TMP_RESULTS_FILE:-}" ]; then
        rm -f "$TMP_RESULTS_FILE"
    fi
}

run_simulation() {
    local -r chunk_size_limit_kb=$1
    local -r config_file=$2
    local -r ns3_cmd_base=$3
    
    TMP_RESULTS_FILE=""
    TMP_RESULTS_FILE=$(mktemp)

    # Save original chunk size config
    local original_chunk_size_line
    original_chunk_size_line=$(grep "ns3::ThreadBlock::chunkSize" "$config_file") || true
    if [ -z "$original_chunk_size_line" ]; then
        echo "Error: Could not find 'ns3::ThreadBlock::chunkSize' in $config_file." >&2
        return 1
    fi
    trap -- 'sed -i "s/^ns3::ThreadBlock::chunkSize .*/'"$original_chunk_size_line"'/" '"$config_file"'' RETURN
    
    # Generate chunk sizes
    local -a chunk_sizes=()
    local size=$START_CHUNK_SIZE_KB
    while [ "$size" -le "$chunk_size_limit_kb" ]; do
        chunk_sizes+=("$size")
        size=$((size * 2))
    done

    local -r ns3_run_cmd="$ns3_cmd_base -- $config_file"

    echo "Chunk Size (KB),Execution Time (s)" > "$TMP_RESULTS_FILE"

    for chunk_size in "${chunk_sizes[@]}"; do
        echo "--> Testing chunk size: ${chunk_size} KB" >&2
        
        # Update config file
        local new_value_bytes=$((chunk_size * 1000))
        sed -i "s/ns3::ThreadBlock::chunkSize [0-9]\+/ns3::ThreadBlock::chunkSize $new_value_bytes/" "$config_file"

        # Measure execution time using time command
        echo "    Executing: $ns3_run_cmd" >&2
        local start_time end_time execution_time
        start_time=$(date +%s.%N)
        NS_LOG="" $ns3_run_cmd > /dev/null 2>&1
        end_time=$(date +%s.%N)
        execution_time=$(awk "BEGIN {print $end_time - $start_time}")

        if ! [[ "$execution_time" =~ ^[0-9.]+$ ]]; then
            echo "Error: Invalid execution time for chunk size ${chunk_size}KB." >&2
            exit 1
        fi
        
        echo "$chunk_size,$execution_time" >> "$TMP_RESULTS_FILE"
        echo "    Execution time: ${execution_time} seconds" >&2
    done

    echo "Simulation runs completed." >&2
}

generate_plot() {
    local -r results_file=$1
    local -r output_plot_path=$2

    if ! command -v gnuplot &> /dev/null; then
        echo "Warning: Gnuplot not found, skipping plot." >&2
        return
    fi

    gnuplot -e "input_files='$results_file'" -e "output_file='$output_plot_path'" -e "title='Execution Time'" -p $GNUPLOT_SCRIPT
    echo "Plot saved to $output_plot_path" >&2
}

# --- Main ---
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
        echo "Error: Chunk size limit (-l) is required." >&2
        usage
    fi

    if ! [[ "$chunk_size_limit_kb" =~ ^[1-9][0-9]*$ ]]; then
        echo "Error: Chunk size limit must be a positive integer." >&2
        exit 1
    fi
    
    if [ ! -f "$config_file" ]; then
        echo "Error: Config file not found: $config_file" >&2
        exit 1
    fi

    mkdir -p "$RESULTS_DIR"

    local -r output_file="${RESULTS_DIR}/${output_prefix}_results.txt"
    local -r output_plot="${RESULTS_DIR}/${output_prefix}_plot.pdf"

    rm -f "$output_file" "$output_plot"

    echo "Starting execution time measurement..." >&2
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
