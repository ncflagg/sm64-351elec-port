#!/bin/sh

set -e

# Create package dir
mkdir ./package

# Copy built content into package dir
cd ./package
mkdir ./sm64
cp -r ../build/us_pc/* ./sm64

# Create 351ELEC launch script
touch ./sm64.sh
cat > ./sm64.sh << EOF
#!/usr/bin/bash

./sm64/sm64.us.f3dex2e

ret_error=\$?

exit \$ret_error
EOF

# Zip the package
zip -r ../../sm64-351ELEC.zip ./*