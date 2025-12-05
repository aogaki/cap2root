#!/bin/bash

echo "=========================================="
echo "COMPREHENSIVE TEST OF convert_all.sh"
echo "=========================================="
echo ""

# Clean up
rm -f *.root

echo "1. Testing file detection"
echo "   - Checking for .cap files..."
cap_files=(*.cap)
echo "   - Found: ${#cap_files[@]} files"
for f in "${cap_files[@]}"; do
    echo "     * $f"
done
echo ""

echo "2. Testing with modified NJOBS=2"
echo "   - Creating test version with NJOBS=2..."
sed 's/NJOBS=10/NJOBS=2/' /Users/aogaki/test/ROSPHER/convert_all.sh > test_convert_all.sh
chmod +x test_convert_all.sh

# Check if cap2root needs to be sourced from ROOT
if ! command -v cap2root &>/dev/null; then
    echo "   - WARNING: cap2root not in PATH"
    echo "   - Modifying script to use build directory cap2root"
    sed -i.bak 's|if cap2root|CAP2ROOT="${CAP2ROOT:-/Users/aogaki/test/ROSPHER/build/cap2root}"\n    if "$CAP2ROOT"|' test_convert_all.sh
    sed -i.bak2 's/cap2root "$input"/$CAP2ROOT "$input"/' test_convert_all.sh
fi

echo ""
echo "3. Running first conversion (should convert all files)..."
echo "=========================================="
./test_convert_all.sh
echo "=========================================="
echo ""

echo "4. Verifying output files..."
echo "   - Checking for .root files..."
root_count=$(ls -1 *.root 2>/dev/null | wc -l | tr -d ' ')
echo "   - Found: $root_count .root files"
ls -lh *.root 2>/dev/null | awk '{print "     *", $9, "("$5")"}'
echo ""

echo "5. Running second conversion (should skip all files)..."
echo "=========================================="
./test_convert_all.sh
echo "=========================================="
echo ""

echo "6. Testing with one file deleted..."
rm -f 152Eu_walk_000001.root
echo "   - Deleted 152Eu_walk_000001.root"
echo "   - Running conversion again..."
echo "=========================================="
./test_convert_all.sh
echo "=========================================="
echo ""

echo "7. Final verification"
root_count=$(ls -1 *.root 2>/dev/null | wc -l | tr -d ' ')
cap_count=${#cap_files[@]}
if [ "$root_count" -eq "$cap_count" ]; then
    echo "   [PASS] All $cap_count .cap files have corresponding .root files"
else
    echo "   [FAIL] Expected $cap_count .root files, found $root_count"
fi
echo ""

echo "=========================================="
echo "COMPREHENSIVE TEST COMPLETE"
echo "=========================================="
