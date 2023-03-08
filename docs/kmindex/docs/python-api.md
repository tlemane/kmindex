# Python API

The python API allows to query a running kmindex server. In the following, we assume that a server is running at 127.0.0.1 on port 8080.

## **Installation**

!!! info "Requirements"
    * python >= 3.10
    * pip (with [PEP-517 support](https://peps.python.org/pep-0517/) if installing from sources)

### Using pip [![pip](https://img.shields.io/pypi/v/pykmindex?logo=pypi)]()

```bash
pip install pykmindex
```

### From sources

```bash
pip install git+https://github.com/tlemane/kmindex.git#subdirectory=pykmindex
```

## **Usage**

### **Connection**

```py
from pykmindex.server import connect

km_serv = connect("127.0.0.1", 8080)

infos = km_serv.infos()
```

### **Query**

#### **Synchronous**

```py
from Bio import SeqIO

from pykmindex.server import connect
from pykmindex.query import Query

km_serv = connect("127.0.0.1", 8080)

for record in SeqIO.parse("query.fa", "fasta"):
    res = km_serv.submit(Query(record.id,
                               record.seq, # Can be a list of sequences
                               "index_id", # Can be a list of sub-indexes to query
                               0.0,        # Min ratio to report a result (optional, default 0.0)
                               3))         # z value (optional, default 3)
    print(res.id, res.result)
```

#### **Asynchronous**

!!! note "Using QBatch"
    ```py
    from Bio import SeqIO
    from pykmindex.server import connect
    from pykmindex.query import QBatch

    km_serv = connect("127.0.0.1", 8080)

    batch = QBatch()

    for record in SeqIO.parse("query.fa", "fasta"):
        batch.add(record.id, record.seq, "index_id")

    responses = km_serv.submit_async(batch)

    for r in responses:
        print(r.id, r.result)
    ```

!!! note "Using your own event loop"
    ```py
    import asyncio
    from Bio import SeqIO
    from pykmindex.server import connect
    from pykmindex.query import Query

    km_serv = connect("127.0.0.1", 8080)

    futures = []
    for record in SeqIO.parse("query.fa", "fasta"):
        futures.append(km_serv.async_query(Query(record.id, record.seq, "index_id")))

    loop = asyncio.get_event_loop()
    responses = loop.run_until_complete(asyncio.gather(*futures))

    for r in responses:
        print(r.id, r.result)
    ```

