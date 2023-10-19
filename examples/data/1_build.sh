#!/bin/bash

kmindex build --fof fof1.txt --run-dir ./D1 --index ./G --register-as D1 --hard-min 1 --kmer-size 25 --bloom-size 1000000
kmindex build --fof fof2.txt --run-dir ./D2 --index ./G --register-as D2 --hard-min 1 --kmer-size 25 --bloom-size 1000000
