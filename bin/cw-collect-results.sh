#!/bin/sh
mkdir ~/out/tmp
mv ~/out/*.out ~/out/tmp/
mv ~/out/*.err ~/out/tmp/
cat out/tmp/* | awk /^\[0-9\].*,\[0-9\].*/ >> ~/results
mv ~/out/tmp/* ~/out/processed
rmdir ~/out/tmp
cat ~/results | sort | uniq > ~/results-tidy
cat ~/results-tidy