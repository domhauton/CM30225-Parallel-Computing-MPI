#!/bin/bash
if [ -d ~/parallel01 ]; then
        rm -rf ~/parallel01
fi

cp -R ~/bucs/Documents/parallel/ ~/parallel01
cd ~/parallel01
cmake CMakeLists.txt
make

if [ -f ~/parallel01/parallel_computation_cw1 ]; then
        mv ~/parallel01/parallel_computation_cw1 ~/
fi

if [ -d ~/parallel01 ]; then
        rm -rf ~/parallel01
fi