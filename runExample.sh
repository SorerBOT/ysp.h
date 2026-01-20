#!/bin/bash

echo "YSP: Compiling..."
gcc ./examples/example.c -rdynamic -g -fno-omit-frame-pointer -o ./bin/example || exit 1

echo "YSP: Finished compiling. Running example:"

./bin/example

echo "Program finished. Results written to profiler.output.txt"
echo "Generating Flame Graph"

./flamegraph.pl profiler.output.txt > profiler.output.svg
echo "Flame Graph stored into profiler.output.svg"
