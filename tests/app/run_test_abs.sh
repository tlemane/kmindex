#!/usr/bin/env bash

kmindex_bin=$1
directory=$2

cd ${directory}

rm -rf out_tmp

${kmindex_bin} query -i indexes/index \
                     -n abs \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f matrix \
                     -o out_tmp 2> /dev/null

diff out_tmp/abs.tsv outputs/q1_z4_abs.tsv || exit 1
rm -rf out_tmp

${kmindex_bin} query -i indexes/index \
                     -n abs \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f json \
                     -o out_tmp 2> /dev/null

diff out_tmp/abs.json outputs/q1_z4_abs.json || exit 1
rm -rf out_tmp

exit 0
