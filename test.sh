#!/bin/sh
set -e
make
for f in testfiles/test_*; do
    ./StreamingService "$f"
done
