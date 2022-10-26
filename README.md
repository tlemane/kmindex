# kmindex (WIP)

kmindex is a tool allowing to perform queries on Bloom filters matrices from [kmtricks](https://github.com/tlemane/kmtricks).

## Installation

### Releases

Portable binary releases are available at [kmindex/releases](https://github.com/tlemane/kmindex/releases).

### From sources

#### Dependencies

* Requires Boost >= 1.70

All other dependencies are bundled with `kmindex`.

```
kmindex build script - v0.0.1.
Usage:
  ./install.sh [-r str] [-t int] [-j int] [-p str] [-n] [-h]
Options:
  -r <Release|Debug> -> build type {Release}.
  -t <0|1|2>         -> tests: 0 = disabled, 1 = compile, 2 = compile and run {2}.
  -j <INT>           -> nb threads {8}.
  -n                 -> portable x86-64 build {disable}.
  -p                 -> install path {./kmindex_install}
  -h                 -> show help.
```

### From conda

```bash
conda create -p kmindex_env
conda activate ./kmindex_env
conda install -c conda-forge -c tlemane/label/dev -c tlemane kmindex
```

## Usage

The [examples](./examples) directory provides a complete example, from index construction to query.


### 1. Build index using kmtricks

See [1_build.sh](./examples/data/1_build.sh).

### 2. Register index

```
kmindex register 0.0.1

DESCRIPTION
  register a new index.

USAGE
  kmindex register --name <STR> --global-index <STR> --index <STR> [-v/--verbose <STR>] [-h/--help] [--version]

OPTIONS
  [global]
     --name         - index name
     --global-index - global index path
     --index        - index path

  [common]
    -h --help    - show this message and exit. [⚑]
       --version - show version and exit. [⚑]
    -v --verbose - verbosity level [debug|info|warning|error]. {info}
```

See [2_register.sh](./examples/data/2_register.sh).

### 3. Query index

```
kmindex query 0.0.1

DESCRIPTION
  query

USAGE
  kmindex query --index <STR> --names <STR> --fastx <STR> [--z <FLOAT>] [--threshold <?>] [--output <STR>]
                [--single-query <STR>] [--format <STR>] [-t/--threads <INT>]
                [-v/--verbose <STR>] [-h/--help] [--version]

OPTIONS
  [global]
     --index        - global index path.
     --names        - indexes to query, comma separated.
     --z            - size of k-mers: (s+z)-mers.  {0}
     --threshold    - shared k-mers threshold. {0}
     --output       - output directory. {output}
     --fastx        - fasta/q file containing the sequence(s) to query.
     --single-query - query id. All sequences are considered as a unique query.
     --format       - output format [json|matrix] {json}

  [common]
    -t --threads - number of threads. {8}
    -h --help    - show this message and exit. [⚑]
       --version - show version and exit. [⚑]
    -v --verbose - verbosity level [debug|info|warning|error]. {info}
```

Outputs are dumped at \<output>/\<index_name>.[json|tsv].
* json output:
```json
{
    "index_1": {
        "1_S1": {
            "D1": 1.0,
            "D2": 0.0
        },
        "1_S2": {
            "D1": 1.0,
            "D2": 0.0
        }
    }
}
```
* matrix output:

```tsv
ID	D1	D2
1_S1	1	0
1_S2	1	0
```

See [3_query.sh](./examples/data/3_query.sh).

## Server

### Start

```
kmindex-server 0.0.1

DESCRIPTION
  kmindex REST server

USAGE
  kmindex-server --index <STR> [--address <STR>] [--port <INT>] [--log-directory <STR>] [--verbose <STR>]
                 [-h/--help]

OPTIONS
  [global] - global parameters
       --index         - index path
       --address       - address {127.0.0.1}
       --port          - port {8080}
       --log-directory - directory for daily logging {kmindex_logs}
       --verbose       - verbosity level [debug|info|warning|error] {info}
    -h --help          - Show this message and exit [⚑]
```

See [4_run_server.sh](./examples/data/4_run_server.sh).

### Query

#### GET request

A GET request allows to obtain informations about indexes.


See [5_get_request.sh](./examples/data/5_get_request.sh).

#### POST request

A POST request allows to query the index.

The body is a json string with 4 entries: `index`, `id`, `seq` and `z`.
* `ìndex`: A json array containing the names of indexes.
* `id`: A query identifier.
* `seq`: A json array containing sequences. All sequences are considered as a single query.
* `z`: z value.

Body example:

```json
{
  "index":[
    "index_1","index_2"
  ],
  "seq":[
    "ACGACGACGACGAGACGAGACGACAGCAGACAGAGACATAATATACTATATAATATATATAGCGAGGGGGGGAGAGCCAGCAGCACCCCCAAAAAAAAA"
  ],
  "id":"1_S1",
  "z":3
}
```

Response example:

```json
{
    "index_1": {
        "1_S1": {
            "D1": 1.0,
            "D2": 0.0
        }
    },
    "index_2": {
        "1_S1": {
            "D3": 0.0,
            "D4": 0.0
        }
    }
}
```


See [6_post_request.sh](./examples/data/6_post_request.sh).


