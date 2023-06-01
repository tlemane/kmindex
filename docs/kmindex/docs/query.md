# Index query

## **kmindex query**

*kmindex query* allows to query all sequences in FASTA/Q file (gz/bz2) against all sub-indexes registered into a global index $G$. For each sequence, the output is a list of either shared $k$-mer ratios (presence/absence mode) or abundance classes (abundance mode).

!!! tip "Options"
    ```
    kmindex query v0.5.2

    DESCRIPTION
      Query index.

    USAGE
      kmindex query -i/--index <STR> -q/--fastx <STR> [-n/--names <STR>] [-z/--zvalue <INT>]
                    [-r/--threshold <FLOAT>] [-o/--output <STR>] [-s/--single-query <STR>]
                    [-f/--format <STR>] [-b/--batch-size <INT>] [-t/--threads <INT>]
                    [-v/--verbose <STR>] [-a/--aggregate] [--fast] [-h/--help] [--version]

    OPTIONS
      [global]
        -i --index        - Global index path.
        -n --names        - Sub-indexes to query, comma separated. {all}
        -z --zvalue       - Index s-mers and query (s+z)-mers (findere algorithm). {0}
        -r --threshold    - Shared k-mers threshold. in [0.0, 1.0] {0.0}
        -o --output       - Output directory. {output}
        -q --fastx        - Input fasta/q file (supports gz/bzip2) containing the sequence(s) to query.
        -s --single-query - Query identifier. All sequences are considered as a unique query.
        -f --format       - Output format [json|matrix|json_vec] {json}
        -b --batch-size   - Size of query batches (0≈nb_seq/nb_thread). {0}
        -a --aggregate    - Aggregate results from batches into one file. [⚑]
           --fast         - Keep more pages in cache (see doc for details). [⚑]

      [common]
        -t --threads - Number of threads. {1}
        -h --help    - Show this message and exit. [⚑]
           --version - Show version and exit. [⚑]
        -v --verbose - Verbosity level [debug|info|warning|error]. {info}
    ```

!!! tip "--single-query <STR\>"
    All sequences in an input file can be considered a unique query by using `--single-query query_name`. The output file contains then shared $k$-mer ratios between the input file and each indexed sample. This corresponds to the query behavior of *kmindex-server* (see [server-query](server-query.md))


!!! warning "--batch-size <INT\>"
    The number of queries in memory is actually `batch-size`$\times$`threads`.


### Presence/Absence query

!!! note "Main parameters"
    * `--index <STR>`: Path to global index.
    * `--fastx <STR>:` File containing queries.
    * `--names <[STR]>`: Sub-indexes to query, comma separated.
    * `--zvalue <INT>`: Usually in $[0,6]$, see [findere algorithm]().
    * `--threshold <FLOAT>`: Report only ratios > `threshold`, in $[0.0,1.0]$.

**Example:** Querying sequence `1` and `2` from `query.fasta` against the presence/absence index $D1$. The [kmindex](https://github.com/tlemane/kmindex) repository offers an `examples` directory where these commands can be tested.

```bash
kmindex query --index ./G --fastx query.fasta --names D1 --zvalue 3 --threshold 0 # (1)!
```

1. Several indexes can be queried at the same time by using `--names D1,D2,...`.

The results are shared ratios between queries and each sample indexed in $D1$. See [Output formats](#output-formats).

#### Output formats

!!! abstract "JSON (`--format json`)"
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
!!! abstract "TSV (`--format matrix`)"
    ```tsv
    D1     S1   S2
    D1:1  1.0  0.0
    D1:2  1.0  0.0
    ```

### Abundance query

The query mode is automatically determined by the index type. The values reported in the output files are simply abundance classes instead of shared $k$-mer ratios.

## About the `z` parameter

 [findere](https://github.com/lrobidou/findere) is a query-time technique allowing to reduce the false positive rate of an arbitrary [AMQF](https://en.wikipedia.org/wiki/Approximate_Membership_Query_Filter) (as Bloom filters) while reducing the number of lookups to resolve a query. Its scope concerns the indexing of objects that are decomposable into sub-objects, i.e. a sequence of characters. It is based on the following straightforward statement: if a sequence $S \in E$, then its sub-words $w_i \in E$.

To benefit from *findere*, small $k$-mers are actually indexed ($s$-mer), and ($s+z$)-mers are considered are query time. As a result, for a given false positive rate of $\epsilon$, the probability of a false match becomes $\epsilon^z$ at query time, reducing the false postive rate by several orders of magnitude. Note that $z$ cannot be arbitrary large, usually $z \in [0, 6]$, see [findere paper](http://dx.doi.org/10.1007/978-3-030-86692-1_13) for more details.

