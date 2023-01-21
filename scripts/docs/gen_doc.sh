#!/bin/bash
sudo apt install -y doxygen

# Move .deb
mkdir -p ../../../louvre_tmp
mkdir -p ../../../louvre_tmp/html

# Get ENV variables
cd ..
chmod +x env.sh
source env.sh
cd ../doxygen

doxygen Doxyfile

cd ..

