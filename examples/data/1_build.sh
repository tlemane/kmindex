#!/bin/bash

kmtricks pipeline --file fof1.txt --run-dir D1 --hard-min 1 --kmer-size 25 --mode hash:bf:bin
kmtricks pipeline --file fof2.txt --run-dir D2 --hard-min 1 --kmer-size 25 --mode hash:bf:bin
