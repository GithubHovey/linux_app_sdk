#!/bin/bash
# 编译环境：WSL2 Ubuntu 22.02
mkdir -p build
cd build 
export LUCKFOX_SDK_PATH="/home/hovey/sdk/luckfox-pico"
cmake ..
make -j4