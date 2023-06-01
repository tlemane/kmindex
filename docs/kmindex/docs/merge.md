# Merge

In addition to registering multiple indexes, compatible sub-indexes (i.e. sharing the parameters: k-mer size, Bloom filter size, number of partitions) can also be merged. This way, the indexes are physically extended resulting in optimal query times.

!!! tip "Options"
    ```
    kmindex merge v0.5.2

    DESCRIPTION
      Merge sub-indexes.

    USAGE
      kmindex merge -i/--index <STR> -n/--new-name <STR> -p/--new-path <STR> -m/--to-merge <LIST[STR]>
                    [-r/--rename <STR>] [-t/--threads <INT>] [-v/--verbose <STR>]
                    [-d/--delete-old] [-h/--help] [--version]

    OPTIONS
      [global]
        -i --index      - Global index path.
        -n --new-name   - Name of the new index.
        -p --new-path   - Output path.
        -m --to-merge   - Sub-indexes to merge, comma separated.
        -d --delete-old - Delete old sub-index files. [⚑]
        -r --rename     - Rename sample ids.
                          - A sub-index cannot contain samples with similar identifiers.
                            Sub-indexes containing identical identifiers cannot be merged, the
                            identifiers must be renamed.
                          - Renaming can be done in three different ways:

                            1. Using identifier files (one per line).
                               For example, if you want to merge three sub-indexes:
                                 '-r f:id1.txt,id2.txt,id3.txt'

                            2. Using a format string ('{}' is replaced by an integer in [0, nb_samples)).
                                 '-r "s:id_{}"'

                            3. Manually (not recommended).
                               Identifiers can be changed in 'kmtricks.fof' files in sub-index directories.

      [common]
        -t --threads - Number of threads. {12}
        -h --help    - Show this message and exit. [⚑]
           --version - Show version and exit. [⚑]
        -v --verbose - Verbosity level [debug|info|warning|error]. {info}
    ```

