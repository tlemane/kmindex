# kmindex

## Installation

### Releases

  Coming soon.

### From sources

#### Dependencies

* Requires Boost >= 1.70

All other dependencies are bundled with `kmindex`.

```bash
git clone --recursive https://github.com/tlemane/kmindex
cd kmindex
mkdir build
cd build
cmake ..
make -j
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

See [2_register.sh](./examples/data/2_register.sh).

### 3. Query index

See [3_query.sh](./examples/data/3_query.sh).

## Server

### Start

See [4_run_server.sh](./examples/data/4_run_server.sh).

### Query

#### GET request

See [5_get_request.sh](./examples/data/5_get_request.sh).

#### POST request

See [6_post_request.sh](./examples/data/5_post_request.sh).


