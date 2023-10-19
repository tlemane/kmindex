#!/bin/bash

echo "Query seq 1_S1 from 1.fasta in D1"
curl -X POST http://127.0.0.1:8080/kmindex/query -H 'Content-type: application/json' -d @./query_1S1_in_index_D1.json

echo

echo "Query seq 1_S1 from 1.fasta in D1 and D2"
curl -X POST http://127.0.0.1:8080/kmindex/query -H 'Content-type: application/json' -d @./query_1S1_in_index_D1_D2.json

echo

echo "Query seq 1_S1 and 1_S2 from 1.fasta in D1 and D2"
echo "1_S1 and 1_S2 are considered as a single query."
curl -X POST http://127.0.0.1:8080/kmindex/query -H 'Content-type: application/json' -d @./query_1S1_1S2_in_index_D1_D2.json

