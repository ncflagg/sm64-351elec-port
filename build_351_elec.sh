#!/bin/sh
JOBS=$(($(nproc)+1))

# Exit build script on error
set -e

make TARGET_RG351=1 TARGET_ELEC=1 -j$JOBS

