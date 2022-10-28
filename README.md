# kmindex

*kmindex* is a tool for querying sequencing samples indexed using [kmtricks](https://github.com/tlemane/kmtricks). 

Given a dataset $D = \{S_1, ..., S_n\}$, it allows to compute the percentage of shared k-mers between a query $Q$ and each $S \in D$. It supports multiple datasets and allows searching for each sub-index $D_i \in G = \{D_1,...,D_2\}$. Indexes are built by [kmtricks](https://github.com/tlemane/kmtricks) (see [section 1. Build](#1.Build)) and does not require additional modifications but have to be registered before performing queries (see [section 2. Register](#2.register)). Queries benefit from the [findere](https://github.com/lrobidou/findere) algorithm. In a few words, *findere* allows to reduce the false positive rate at query time by querying $(s+z)$-mers instead of $s$-mers, which are the indexed words, usually called $k$-mers (see [section 3. Query](#3.Query)).

For easy integration, *kmindex* is also composed of a server supporting http requests (see [section Server](#Server)).


Note that *kmindex* is a work in progress, its features and usages may be subject to change.


## Installation

### Releases

Portable binary releases are available at [kmindex/releases](https://github.com/tlemane/kmindex/releases).

### From sources

#### Dependencies

* Requires Boost.Asio and Boost.System >= 1.70 and bzip2

All other dependencies are bundled with `kmindex`.

```
kmindex build script - v0.1.0.
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
conda install -c conda-forge -c tlemane kmindex
```

Note that the conda package includes [kmtricks](https://github.com/tlemane/kmtricks)

## Usage

The [examples](./examples) directory provides a complete example, from index construction to queries, including scripts and data for easy testing.


### 1. Build

Indexes are built by *kmtricks* (not bundled by *kmindex*, see [kmtricks installation](https://github.com/tlemane/kmtricks/wiki/Installation)). To be usable by *kmindex*, the index has to be constructed with `--mode hash:bf:bin` and without `--cpr`. The $k$-mer size should also not exceed $31$. The other flags can be used as usual (see [kmtricks pipeline](https://github.com/tlemane/kmtricks/wiki/kmtricks-pipeline)).

**Example:**

```bash
kmtricks pipeline --file fof1.txt --run-dir D1 --hard-min 1 --kmer-size 25 --mode hash:bf:bin
kmtricks pipeline --file fof2.txt --run-dir D2 --hard-min 1 --kmer-size 25 --mode hash:bf:bin
```

### 2. Register index

`kmindex register` allows to register a *kmtricks* index $D$ into a global index $G$.

```
kmindex register v0.1.0

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

**Example:**

```bash
kmindex register --name D1 --global-index G --index ./index_1
kmindex register --name D2 --global-index G --index ./index_2
```

### 3. Query index

`kmindex query` allows to query a fastx file against one or more indexes from a global index $G$.

```
kmindex query v0.1.0

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

**Example:**

Assuming a fasta file containing two sequences $1$ and $2$, and querying $D1$ (containing two samples $S1$ and $S2$) from $G$, two outputs are available:

```bash
kmindex query --index ./G --names D1 --output output --fastx 1.fasta --format [json|matrix]
```

output/D1.json:
```json
{
    "D1": {
        "1": {
            "S1": 1.0,
            "S2": 0.0
        },
        "2": {
            "S1": 1.0,
            "S2": 0.0
        }
    }
}
```

output/D1.tsv:
```tsv
ID	S1	S2
1	1	0
2	1	0
```

## Server

kmindex-server allows to obtain informations about the index via GET requests and to perform queries via POST requests.

```
kmindex-server v0.1.0

DESCRIPTION
  kmindex-server allows to perform queries via POST requests.

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

### 1. Start

```bash
kmindex-server --index ./G --address 127.0.0.1 --port 8080
```
Note that you should use `--address ""` to bind any address if you want listening from outside.

### 2. GET

GET requests must be sent to /kmindex/infos

```bash
curl -X GET http://127.0.0.1:8080/kmindex/infos
```

**Response:**
```
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

### 3. POST

POST requests must be sent to /kmindex/query

The body a is json string with 4 entries:

* 'index': an array of strings corresponding to the indexes to query.
* 'id': a string used as query identifier.
* 'z': a integer which determine the $k$-mer size, $(s+z)$-mers.
* 'seq': an array of strings corresponding to the sequences to query, which are considered as a singe query (same as `kmindex query --single-query `).

```bash
curl -X POST http://127.0.0.1:8080/kmindex/query -H 'Content-type: application/json' \
     -d '{"index":["D1"],"seq":["ACGACGACGACGAGACGAGACGACAGCAGACAGAGACATAATATACT"], "id":"ID","z":3}'

```
**Response:**
```
{
    "D1": {
        "ID": {
            "S1": 1.0,
            "S2": 0.0
        },
    }
}
```

