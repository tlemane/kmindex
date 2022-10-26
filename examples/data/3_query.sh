#!/bin/bash

kmindex query --index ./gindex --names index_1,index_2 --output output_1 --format json --fastx 1.fasta
kmindex query --index ./gindex --names index_1,index_2 --output output_2 --format json --fastx 2.fasta
kmindex query --index ./gindex --names index_1,index_2 --output output_3 --format json --fastx 3.fasta
kmindex query --index ./gindex --names index_1,index_2 --output output_4 --format json --fastx 4.fasta
