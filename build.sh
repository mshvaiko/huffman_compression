#!/bin/sh

# this library uses tbb
sudo apt install libtbb-dev

mkdir build && cd build
cmake .. && cmake --build .
