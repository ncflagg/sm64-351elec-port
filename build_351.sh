#!/bin/sh
JOBS=$(($(nproc)+1))

# Exit build script on error
set -e

rm -r ./build
make TARGET_RG351=1 -j$JOBS
