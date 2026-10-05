# Index query (many sub-indexes)

## **kmindex query2**

*kmindex query2* is an alternative to [*kmindex query*](query.md) intended for global indexes registering **many sub-indexes**, i.e. hundreds or thousands. Both commands produce the same results and support the same output formats, they only differ in the way the work is distributed:

* *kmindex query* processes sub-indexes one after the other, and parallelizes over **batches of query sequences** within a sub-index.
* *kmindex query2* loads all query sequences once, then parallelizes over **sub-indexes**: `--threads` sub-indexes are queried concurrently, each one producing its own result file.

!!! tip "Options"
    ```
    kmindex query2 v0.7.0

    DESCRIPTION
      To be used instead of kmindex query when many sub-indexes are registered, i.e. hundreds or thousands.

    USAGE
      kmindex query2 -i/--index <STR> -q/--fastx <STR> [-n/--names <STR>] [-z/--zvalue <INT>]
                     [-r/--threshold <FLOAT>] [-o/--output <STR>] [-f/--format <STR>]
                     [--memory-budget <INT>] [-t/--threads <INT>] [-v/--verbose <STR>] [--fast]
                     [--merge] [-h/--help] [--version]

    OPTIONS
      [global]
        -i --index         - Global index path. Multiple comma-separated paths are merged (sub-indexes from all paths are queried together).
        -n --names         - Sub-indexes to query, comma separated. {all}
        -z --zvalue        - Index s-mers and query (s+z)-mers (findere algorithm). {0}
        -r --threshold     - Shared k-mers threshold. in [0.0, 1.0] {0.0}
        -o --output        - Output directory. {output}
        -q --fastx         - Input fasta/q file (supports gz/bzip2) containing the sequence(s) to query.
        -f --format        - Output format [json|matrix|json_vec|jsonl|jsonl_vec] {json}
           --fast          - Keep more pages in cache (see doc for details). [⚑]
           --memory-budget - Total memory budget for concurrent sub-index queries in MB (heap + mmap working set). 0 = no limit. {0}
           --merge         - Write a single merged result file ({output}/merged.{ext}) instead of one file per sub-index. [⚑]

      [common]
        -t --threads - Number of threads. {1}
        -h --help    - Show this message and exit. [⚑]
           --version - Show version and exit. [⚑]
        -v --verbose - Verbosity level [debug|info|warning|error]. {info}
    ```

!!! note "Differences with `kmindex query`"
    *kmindex query2* has no `--batch-size`/`--batch-size-base`, no `--aggregate` and no `--single-query`. All query sequences are kept in memory and each sub-index is written to its own file in `--output`, named after the sub-index (unless `--merge` is used, see below).

### Usage

```bash
kmindex query2 --index ./G --fastx query.fasta --zvalue 3 --format matrix --threads 8
```

Results are written to `output/<sub-index>.tsv` (one file per queried sub-index, or a single `output/merged.tsv` with `--merge`), with the same content as the corresponding *kmindex query* run. See [Output formats](query.md#output-formats).

#### Selecting sub-indexes

As with *kmindex query*, `--names` restricts the query to a subset of sub-indexes. When the list becomes long, it can be read from a file (one sub-index name per line) by prefixing the path with `@`:

```bash
kmindex query2 -i ./G -q query.fasta -n D1,D2,D3     # inline list
kmindex query2 -i ./G -q query.fasta -n @names.txt   # one name per line
```

Querying a name that is not registered in the global index is an error. So is a missing or empty `@names` file — the file must exist and contain at least one name. Names must be single filename components: they cannot contain `/` or be `.`/`..`.

#### Querying several global indexes (requires >= v0.7.0)

`--index` accepts several global index paths, comma separated. They are merged in memory before the query, and the sub-indexes of all paths are queried together as if they belonged to a single global index:

```bash
kmindex query2 --index ./G1,./G2 --fastx query.fasta --format matrix
```

The merge is done in memory only, the indexes on disk are left untouched. `--names` applies to the merged set.

!!! warning "Duplicated sub-index names"
    Merged global indexes must not register two sub-indexes with the same name. A name collision aborts the query with `Sub-index name collision: '<name>' exists in multiple --index paths`.

#### Limiting memory usage (requires >= v0.7.0)

Since sub-indexes are queried concurrently, peak memory grows with `--threads`. `--memory-budget <MB>` caps the total memory used by the concurrent queries: *kmindex* estimates the peak memory of each sub-index query (from the number of samples, the number of $s$-mers in the query file and the output format), then admits a new sub-index only when it fits within the remaining budget. Sub-indexes are scheduled largest-first.

```bash
kmindex query2 --index ./G --fastx query.fasta --threads 16 --memory-budget 32000
```

With `--memory-budget 0` (default), no limit is applied and all `--threads` queries run concurrently.

!!! note
    The budget throttles concurrency, it never changes the results. A sub-index whose own estimated requirement exceeds the budget is not skipped, it runs alone, and a warning is emitted.

#### Merging results into a single file (requires >= v0.7.0)

`--merge` writes a single result file, `output/merged.{ext}`, instead of one file per sub-index. The content is the union of the per-index files — only the file layout changes, never the results:

* **json / json_vec**: one document with a top-level key per sub-index. A sub-index with no hit still appears as `"<sub-index>": {}` (or `"<sub-index>": {"<query>": {}}` when queries were run but all their hits were filtered by `--threshold`), matching the per-index output.
* **jsonl / jsonl_vec**: the per-index files concatenated — one line per `(index, query)` pair.
* **matrix**: the per-index blocks concatenated, each retaining its own header line.

```bash
kmindex query2 --index ./G1,./G2 --fastx query.fasta --format matrix --merge
# writes output/merged.tsv
```

Sub-indexes are emitted in a deterministic order: the `--names` order when an explicit list is given, lexicographic sub-index name order otherwise. The merged file is assembled after all sub-index queries complete, using per-index fragments staged under `output/.staging` (removed on success); while assembling, roughly the final file size is needed as extra disk space. The file is written through a temporary name and renamed only once complete, so a failed run never leaves a partial `merged.*` — it exits with a non-zero status instead.
