#!/bin/bash

cd ../../../src
meson setup build -Dbuildtype=release
cd build
sudo meson install
cd ../../scripts/packaging/deb


