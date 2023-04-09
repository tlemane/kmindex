# Index construction

## **kmindex build**

`kmindex build` allows to construct an index $I$ from a set of FASTA/Q files (gzipped or not) and register it into a global index $G$. It supports both presence/absence and abundance indexing.

!!! note
    `kmindex build` is basically a wrapper around [kmtricks](https://github.com/tlemane/kmtricks), ensuring indexes are built in the right way. However, indexes built by `kmtricks` are usable by `kmindex` if built using `--mode hash:bf:bin` (presence/absence mode) or `--mode hash:bfc:bin` (abundance mode), and without `--cpr`. Other flags can be used as usual (see [kmtricks pipeline](https://github.com/tlemane/kmtricks/wiki/kmtricks-pipeline)). To be queryable, indexes have to be registered into a global index (see `kmindex register`).

!!! tip "Options"
    ```
    kmindex build v0.4.0

    DESCRIPTION
      Build index.

    USAGE
      kmindex build -i/--index <STR> -f/--fof <STR> -d/--run-dir <STR> -r/--register-as <STR> [--from <STR>]
                    [--km-path <STR>] [-k/--kmer-size <INT>] [-m/--minim-size <INT>]
                    [--hard-min <INT>] [--nb-partitions <INT>] [--bloom-size <INT>]
                    [--nb-cell <INT>] [--bitw <INT>] [-t/--threads <INT>] [-v/--verbose <STR>]
                    [--cpr] [-h/--help] [--version]

    OPTIONS
      [global]
        -i --index       - Global index path.
        -f --fof         - kmtricks input file.
        -d --run-dir     - kmtricks runtime directory.
        -r --register-as - Index name.
           --from        - Use parameters from a pre-registered index.
           --km-path     - Path to kmtricks binary.
                           - If empty, kmtricks is searched in $PATH and
                             at the same location as the kmindex binary.

      [kmtricks parameters] - See kmtricks pipeline --help
        -k --kmer-size     - Size of a k-mer. in [8, 31] {31}
        -m --minim-size    - Size of minimizers. in [4, 15] {10}
           --hard-min      - Min abundance to keep a k-mer. {2}
           --nb-partitions - Number of partitions (0=auto). {0}
           --cpr           - Compress intermediate files. [⚑]

      [presence/absence indexing]
         --bloom-size - Bloom filter size.

      [abundance indexing]
         --nb-cell - Number of cells in counting Bloom filter.
         --bitw    - Number of bits per cell. {2}
                     - Abundances are indexed by log2 classes (nb classes = 2^{bitw})
                       For example, using --bitw 3 resulting in the following classes:
                         0 -> 0
                         1 -> [1,2)
                         2 -> [2,4)
                         3 -> [4,8)
                         4 -> [8,16)
                         5 -> [16,32)
                         6 -> [32,64)
                         7 -> [64,max(uint32_t))

      [common]
        -t --threads - Number of threads. {12}
        -h --help    - Show this message and exit. [⚑]
           --version - Show version and exit. [⚑]
        -v --verbose - Verbosity level [debug|info|warning|error]. {info}
    ```

!!! note "`--from <STR>`"
    Indexes build using parameters from a pre-registered index can be merged. See [`kmindex merge`](merge.md).


### **Input file format**

One sample per line, with a unique ID, and a list of FASTA/Q files separated by semicolons.

**Example**
```
D1: /path/to/D1_1.fasta ; /path/to/D1_2.fasta ; /path/to/D1_3.fasta
D2: /path/to/D2_1.fasta.gz ; /path/to/D2_2.fasta.gz
```

An example on how to get such an input fof from a folder containing many input files is:
```bash
ls -1 folder/*  | sort -n -t/ -k 2 |awk '{print ++count" : "$1}' > fof.txt
```

### **Presence/Absence indexing**

In presence/absence mode, the index only contains the presence/absence pattern of $k$-mers in the input samples. As a result, a query response corresponds to shared $k$-mer ratios between the sequence and each sample contained in the index.

!!! note "Required parameters"
    * `--index`: Path to the global index.
    * `--fof`: Input file, see [input file](#Input-file-format).
    * `--run-dir`: Index output path.
    * `--register-as`: Name of the index.
    * `--bloom-size`: Number of bits in Bloom filters.

**Examples**

The [kmindex](https://github.com/tlemane/kmindex) repository offers an `examples` directory where these commands can be tested.
```bash
kmindex build --fof fof1.txt --run-dir D1_index --index ./G --register-as D1 --hard-min --kmer-size 25 --bloom-size 1000000 # (1)!
kmindex build --fof fof2.txt --run-dir D2_index --index ./G --register-as D2 --hard-min --kmer-size 25 --bloom-size 1000000 # (2)!
```

1. Creates a directory `G` containing the index for the `D1` dataset
2. Add the index for the `D2` dataset in the previously created `G` directory


### **Abundance indexing**

In abundance mode, the index associates each $k$-mer to an abundance class in each sample. The abundance class is computed using log2($k$-mer abundance). The number of classes depends on the number of bits (`--bitw`) used to encode them ($2^{bitw}$).

As a result, a query response corresponds to an abundance class which is the average of all $k$-mer classes in the query sequence, rounded down to the lowest class.

!!! note "Required parameters"
    * `--index`: Path to the global index.
    * `--fof`: Input file, see [input file](#Input-file-format).
    * `--run-dir`: Index output path.
    * `--register-as`: Name of the index.
    * `--nb-cell`: Number of cells in Counting Bloom filters.
    * `--bitw`: Number of bits per cell (nb classes = $2^{bitw}$).

**Examples**

```bash
kmindex build --fof fof1.txt --run-dir D1_index --index ./G --register-as D1 --hard-min --kmer-size 25 --nb-cell 1000000 --bitw 2 # (1)!
kmindex build --fof fof2.txt --run-dir D2_index --index ./G --register-as D2 --hard-min --kmer-size 25 --nb-cell 1000000 --bitw 4 # (2)!
```

1. D1 is indexed using 4 abundance classes.
2. D2 is indexed using 16 abundance classes.

## **About Bloom filter size**

*kmindex* relies on Bloom filters, a probabilistic data structure exposed to false postives. The false positive rate, $\epsilon$, depends the number of bits in the Bloom filter, the number of elements to insert (the number of $k$-mers) and the number of hash functions (always **1** here). The cardinality of your samples can be quickly estimated using [ntCard](https://github.com/bcgsc/ntCard). The Bloom filter size should then be choosen according to the sample containing the largest number of $k$-mers.

!!! tip
    Some online tools can help: [https://hur.st/bloomfilter/](https://hur.st/bloomfilter/)

Note that *kmindex* benefits from the [findere algorithm](https://github.com/lrobidou/findere), a query-time technique allowing to drastically reduce the false positive rates. An approximation of the false positive rate under *findere* is $\epsilon^z$, where $z$ is a query-time parameter (which cannot be arbitrary large, see [z help](query.md#about-the-z-parameter) and [findere paper](http://dx.doi.org/10.1007/978-3-030-86692-1_13)). It is recommended to choose your filter size according to this point.

