#!/bin/bash

echo "Query seq 1_S1 from 1.fasta in index_1"
curl --compressed -X POST http://127.0.0.1:8080/kmindex/query -H 'Content-type: application/json' -d @./query_1S1_in_index_1.json

echo

echo "Query seq 1_S1 from 1.fasta in index_1 and index_2"
curl --compressed -X POST http://127.0.0.1:8080/kmindex/query -H 'Content-type: application/json' -d @./query_1S1_in_index_1_2.json

echo

echo "Query seq 1_S1 and 1_S2 from 1.fasta in index_1 and index_2"
echo "1_S1 and 1_S2 are considered as a single query."
curl -X POST http://127.0.0.1:8080/kmindex/query -H 'Content-type: application/json' -d @./query_1S1_1S2_in_index_1_2.json

