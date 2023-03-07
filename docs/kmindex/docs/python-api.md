# Python API

The python API allows to query a running kmindex server. In the following, we assume that a server is running at 127.0.0.1 on port 8008.

## **Installation**

!!! info "Requirements"
    * python >= 3.x
    * pip (with [PEP-517 support](https://peps.python.org/pep-0517/) if installing from sources)

### Using pip [![pip](https://img.shields.io/pypi/v/pykmindex?logo=pypi)]()

```bash
pip install pykmindex
```

### From sources

```bash
pip install git+https://github.com/tlemane/kmindex.git#subdirectory=pykmindex
```

## Usage

### Connection

```py
from pykmindex import connect

km_serv = connect("127.0.0.1", 8080)

infos = km_serv.infos()
```

### Query

#### Sync

```py
from pykmindex import Query

q = Query(indexes=["index1", "index2"],
          name="Q1",
          seqs=["AGCAGTACTAACAA", "CAGCAGATACTCAAGAGCA"],
          r=0.5
          z=3)

# res is a dictionnary corresponding to the json response
res = km_serv.submit(q)
```


#### Async

```py
from Bio import SeqIO
from pykmindex import QBatch

batch = QBatch()

indexes = ["index1", "index2"]

for record in SeqIO.parse("query.fa", "fasta"):
  batch.add(indexes=indexes, name=record.id, seqs=[record.seq], r=0.5, z=3)

# res is dictionnary where query identifiers are associated to the json responses.
res = km_serv.submit(batch)
```
