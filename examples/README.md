Here is an example of how to use `kmindex`. We build two indexes, `D1` and `D2`, each containing 2 samples. These indexes are registered in a global index `G`. The `data` directory contains data and scripts to easily execute the commands presented here.

## CLI Examples

### Construction

```bash
# Constructs D1 and registers it in G
kmindex build --fof fof1.txt --run-dir ./D1 --index ./G --register-as D1 --hard-min 1 --kmer-size 25 --bloom-size 1000000
# Constructs D2 and registers it in G
kmindex build --fof fof2.txt --run-dir ./D2 --index ./G --register-as D2 --hard-min 1 --kmer-size 25 --bloom-size 1000000
```

### Query

```bash
# Query D1 and D2 in G, output in json format
kmindex query --index ./G --names D1,D2 --output output_1 --format json --fastx 1.fasta
kmindex query --index ./G --names D1,D2 --output output_2 --format json --fastx 2.fasta
kmindex query --index ./G --names D1,D2 --output output_3 --format json --fastx 3.fasta
kmindex query --index ./G --names D1,D2 --output output_4 --format json --fastx 4.fasta
```

## Server Examples

### Start the server

```bash
kmindex-server --index ./G
```

### Get index informations
```bash
curl -X GET http://127.0.0.1:8080/kmindex/infos
```

### Query

```bash
# Query seq 1_S1 from 1.fasta in D1"
curl -X POST http://127.0.0.1:8080/kmindex/query -H 'Content-type: application/json' -d @./query_1S1_in_index_D1.json

# Query seq 1_S1 from 1.fasta in D1 and D2"
curl -X POST http://127.0.0.1:8080/kmindex/query -H 'Content-type: application/json' -d @./query_1S1_in_index_D1_D2.json

# Query seq 1_S1 and 1_S2 from 1.fasta in D1 and D2"
# 1_S1 and 1_S2 are considered as a single query
curl -X POST http://127.0.0.1:8080/kmindex/query -H 'Content-type: application/json' -d @./query_1S1_1S2_in_index_D1_D2.json
```
