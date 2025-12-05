#!/bin/bash

# Parallel cap to ROOT converter
# Converts all .cap files in current directory to .root format
# Uses 10 parallel threads for conversion

# Number of parallel jobs
NJOBS=10

# Determine cap2root location
# First check if cap2root is available and working
CAP2ROOT="cap2root"
if ! command -v cap2root &> /dev/null; then
    # Not in PATH, try build directory
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    if [ -x "$SCRIPT_DIR/build/cap2root" ]; then
        CAP2ROOT="$SCRIPT_DIR/build/cap2root"
    fi
else
    # Test if the cap2root in PATH actually works
    if ! cap2root --help &> /dev/null 2>&1; then
        # System cap2root has issues, try build directory
        SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
        if [ -x "$SCRIPT_DIR/build/cap2root" ]; then
            CAP2ROOT="$SCRIPT_DIR/build/cap2root"
        fi
    fi
fi

# Find all .cap files in current directory
cap_files=(*.cap)

# Check if any .cap files exist
if [ ! -e "${cap_files[0]}" ]; then
    echo "No .cap files found in current directory"
    exit 1
fi

echo "Found ${#cap_files[@]} .cap files"
echo "Using cap2root: $CAP2ROOT"
echo "Starting conversion with $NJOBS parallel threads..."
echo ""

# Function to convert a single file
convert_file() {
    local input="$1"
    local output="${input%.cap}.root"

    # Check if output already exists
    if [ -f "$output" ]; then
        echo "[SKIP] $input -> $output (already exists)"
        return 0
    fi

    echo "[START] Converting $input..."

    # Run conversion
    if "$CAP2ROOT" "$input" "$output" > /dev/null 2>&1; then
        echo "[DONE] $input -> $output"
    else
        echo "[ERROR] Failed to convert $input"
        return 1
    fi
}

# Export function so it can be used by parallel processes
export -f convert_file
export CAP2ROOT

# Use GNU parallel if available, otherwise use xargs
if command -v parallel &> /dev/null; then
    # GNU parallel method (preferred)
    printf '%s\n' "${cap_files[@]}" | parallel -j $NJOBS convert_file
else
    # Fallback using xargs with background jobs
    printf '%s\n' "${cap_files[@]}" | xargs -n 1 -P $NJOBS -I {} bash -c 'convert_file "$@"' _ {}
fi

echo ""
echo "Conversion complete!"
echo "Summary:"
echo "  Total .cap files: ${#cap_files[@]}"
echo "  ROOT files created: $(ls -1 *.root 2>/dev/null | wc -l)"
