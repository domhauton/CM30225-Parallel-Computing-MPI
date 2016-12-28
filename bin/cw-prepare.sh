#!/bin/bash
if [ -d ~/parallel02 ]; then
        rm -rf ~/parallel02
fi

cp -R ~/bucs/Documents/parallel-2/ ~/parallel02
cd ~/parallel02
cmake CMakeLists.txt
make

if [ -d ~/parallel02 ]; then
        rm -rf ~/parallel02
fi