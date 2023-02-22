# Build image

```bash
docker build -t kmindex-d https://github.com/tlemane/kmindex.git#dev:docker

```
# Run

```bash
docker run --rm kmindex-d <kmindex args>
```

The default entrypoint corresponds to `kmindex`. To run `kmindex-server`, use:

```bash
docker run --rm --entrypoint /result/bin/kmindex-server kmindex-d <kmindex-server args>
```
