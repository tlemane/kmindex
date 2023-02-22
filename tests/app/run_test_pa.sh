#!/usr/bin/env bash

kmindex_bin=$1
directory=$2

cd ${directory}

rm -rf out_tmp

${kmindex_bin} query -i indexes/index \
                     -n pa \
                     -q datasets/pa_dataset/1.fasta \
                     -z 4 \
                     -f matrix \
                     -z 5 \
                     -o out_tmp 2> /dev/null

diff out_tmp/pa.tsv outputs/q1_z5_pa.tsv || exit 1
rm -rf out_tmp

${kmindex_bin} query -i indexes/index \
                     -n pa \
                     -q datasets/pa_dataset/1.fasta \
                     -z 4 \
                     -f json \
                     -r 0.5 \
                     -o out_tmp 2> /dev/null

diff out_tmp/pa.json outputs/q1_pa.json || exit 1
rm -rf out_tmp

exit 0
