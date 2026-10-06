#!/usr/bin/env bash

kmindex_bin=$1
directory=$2

cd ${directory}

${kmindex_bin} compress -i indexes/index -n pa -r --check

rm indexes/pa_index/compression.cfg
rm indexes/pa_index/matrices/blocks*
rm indexes/pa_index/permutations.bin
mv indexes/pa_index/kmtricks.fof.bak indexes/pa_index/kmtricks.fof

git restore ./indexes



