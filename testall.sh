#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
shopt -s nullglob

# inputs
dir=$1
logdir=$2

# settings
if [[ "$OSTYPE" == "darwin"* ]]; then
    MAX_JOBS=$(sysctl -n hw.ncpu) # Mac
else
    MAX_JOBS=$(nproc) # Linux
fi
CHECK_INTERVAL=1

# main process
for eachdir in "$dir"/*; do
    [ -d "$eachdir" ] || continue

    group=$(basename "$eachdir")
    mkdir -p "${logdir}/${group}"

    for file in "$eachdir"/*; do
        [ -f "$file" ] || continue

        while (( $(jobs -rp | wc -l) >= MAX_JOBS )); do
            sleep $CHECK_INTERVAL
        done

        filename=$(basename "$file")
        base="${filename%.*}"
        log_file="${logdir}/${group}/${base}.log"

        echo "Processing $file to $log_file"
        ./build/a.out -i "$file" > "$log_file" &
    done
done

# wait for all background jobs to finish
wait
echo "All jobs completed."
