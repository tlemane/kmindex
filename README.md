# kmindex (WIP)

## Rationale

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
kmindex register v0.0.1

DESCRIPTION
  Register index.

USAGE
  kmindex register --global-index <STR> --name <STR> --index <STR> [-v/--verbose <STR>] [-h/--help] [--version]

OPTIONS
  [global]
     --global-index - Global index path.
     --name         - Index name.
     --index        - Index path (a kmtricks run using --mode hash:bf:bin).

  [common]
    -h --help    - Show this message and exit. [⚑]
       --version - Show version and exit. [⚑]
    -v --verbose - Verbosity level [debug|info|warning|error]. {info}
```

See [2_register.sh](./examples/data/2_register.sh).

### 3. Query index

```
kmindex query v0.0.1

DESCRIPTION
  Query index.

USAGE
  kmindex query -i/--index <STR> -n/--names <STR> -q/--fastx <STR> [-z/--zvalue <INT>] [-r/--threshold <?>]
                [-o/--output <STR>] [-s/--single-query <STR>] [-f/--format <STR>]
                [-t/--threads <INT>] [-v/--verbose <STR>] [-h/--help] [--version]

OPTIONS
  [global]
    -i --index        - Global index path.
    -n --names        - Sub-indexes to query, comma separated.
    -z --zvalue       - Index s-mers and query (s+z)-mers (findere algorithm). {0}
    -r --threshold    - Shared k-mers threshold (currently unused). {0}
    -o --output       - Output directory. {output}
    -q --fastx        - Input fasta/q file (supports gz/bzip2) containing the sequence(s) to query.
    -s --single-query - Query identifier. All sequences are considered as a unique query.
    -f --format       - Output format [json|matrix] {json}

  [common]
    -t --threads - Number of threads. {8}
    -h --help    - Show this message and exit. [⚑]
       --version - Show version and exit. [⚑]
    -v --verbose - Verbosity level [debug|info|warning|error]. {info}
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
kmindex-server v0.0.1

DESCRIPTION
  kmindex-server allows to perform queries via POST requests.

  Examples:

     curl --post302 -L -X http://127.0.0.1:8080/kmindex/query -H 'Content-type: application/json'
          -d '{"index":["index_1"],"seq":["AGAGCCAGCAGCACCCCCAAAAAAAAA"],
          "id":"ID1","z":3}'

     curl -L -X http://127.0.0.1:8080/kmindex/infos

USAGE
  kmindex-server -i/--index <STR> [-a/--address <STR>] [-p/--port <INT>] [-d/--log-directory <STR>]
                 [-t/--threads <INT>] [--verbose <STR>] [-s/--no-stderr] [-h/--help]
                 [--version]

OPTIONS
  [global] - global parameters
    -i --index         - Index path.
    -a --address       - Address to use (empty string to bind any address)
                             IPv4: dotted decimal form
                             IPv6: hexadimal form.
                             default ->  {127.0.0.1}
    -p --port          - Port to use. {8080}
    -d --log-directory - Directory for daily logging. {kmindex_logs}
    -s --no-stderr     - Disable stderr logging. [⚑]

  [common]
    -t --threads - Max number of parallel connections. {1}
    -h --help    - Show this message and exit. [⚑]
       --version - Show version and exit. [⚑]
       --verbose - Verbosity level [debug|info|warning|error]. {info}

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


