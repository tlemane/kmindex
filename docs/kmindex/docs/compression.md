# Index compression

Since version `v0.6.0`, `kmindex` allows to make compressed indexes.
It works by dividing an index into blocks, which are then compressed using [`BlockCompressor`](https://github.com/AlixRegnier/BlockCompressor). At query time, only the relevant blocks are decompressed.


## **kmindex compress** (requires >= v0.6.0)

!!! tip "Options"
    ```
    kmindex compress v0.6.0

    DESCRIPTION
      Compress index.

    USAGE
      kmindex compress -i/--global-index <STR> -n/--name <STR> [-b/--block-size <INT>] [-s/--sampling <INT>]
                       [-c/--column-per-block <INT>] [-l/--cpr-level <INT>]
                       [-t/--threads <INT>] [-v/--verbose <STR>] [-d/--delete] [--check]
                       [-r/--reorder] [-h/--help] [--version]

    OPTIONS
      [global]
        -i --global-index - Global index path.
        -n --name         - Index name.
        -d --delete       - Delete uncompressed index after compressing. [⚑]
           --check        - Check query results after compressing. [⚑]

      [Reordering options]
        -b --block-size       - Size of uncompressed blocks, in megabytes. {8}
        -r --reorder          - Reorder columns before compressing. [⚑]
        -s --sampling         - Number of rows to sample for reordering. {20000}
        -c --column-per-block - Reorder columns by group of N. Should be a multiple of 8 (0=all) {0}
        -l --cpr-level        - Compression level in [1,22]) {6}

      [common]
        -t --threads - Number of threads. {22}
        -h --help    - Show this message and exit. [⚑]
           --version - Show version and exit. [⚑]
        -v --verbose - Verbosity level [debug|info|warning|error]. {info}
    ```

### Usage

#### Basic compression

The following command compress the index `index_1` registered in the global index `global_index` (built using `kmindex build`, or built with `kmtricks` and registered with `kmindex register`, see [Index construction](construction.md)).


```bash
kmindex compress -i ./global_index -n index_1
```





