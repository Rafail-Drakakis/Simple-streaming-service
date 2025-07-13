#!/bin/sh
set -e
make
for f in testfiles/test_*; do
    ./cs240StreamingService "$f"
done
