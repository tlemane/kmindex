# Server query

In the following, we assume that a *kmindex-server* is running at 127.0.0.1 on port 8080.


| Method | Description | Path |
| ------ | ----------- | ---  |
| `GET`  | Index informations | /kmindex/infos |
| `POST` | Index query | /kmindex/query |

## **Accessing index information**

**Request**
```bash
curl -X GET http://127.0.0.1:8080/kmindex/infos
```

**Response**
```json
{
    "index": {
        "D1": {
            "bloom_size": 10000128,
            "index_size": 9,
            "minim_size": 10,
            "nb_partitions": 4,
            "nb_samples": 2,
            "samples": [
                "S1",
                "S2"
            ],
            "smer_size": 25
        },
        "D2": {
            "bloom_size": 10000128,
            "index_size": 9,
            "minim_size": 10,
            "nb_partitions": 4,
            "nb_samples": 2,
            "samples": [
                "S3",
                "S4"
            ],
            "smer_size": 25
        }
    }
```

## Query index

!!! note "json body"
    * `index`: An array of strings corresponding to the indexes to query.
    * `id`: A string used as query identifier.
    * `seq`: An array of strings corresponding to the sequences to query. All sequences are considered as a unique query (same as `kmindex query --single-query <STR>`)
    * `format`: A string in ["json", "tsv"] corresponding to the output format.
    * `z`: The z parameter, see [z help](query.md#about-the-z-parameter) (default: 0).
    * `r`: A float in $[0,1]$, all ratios greater than `r` are reported (default: 0).

**Request**

```bash
curl -X POST http://127.0.0.1:8080/kmindex/query \
     -H 'Content-type: application/json' \
     -d '{"index":["D1"],"seq":["ACGACGACGACGAGACGAGACGACAGCAGACAGAGACATAATATACT"], "id":"ID","z":3} #(1)!'
```
1. To avoid curl to change POST requests to GET on redirection, use `--post302`.

**Response**

```json
{
    "D1": {
        "ID": {
            "S1": 1.0,
            "S2": 0.0
        },
    }
}
```



```
