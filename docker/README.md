# Build image

```bash
git clone https://github.com/tlemane/kmindex.git
cd kmindex/docker
docker build -t kmindex-d .
```

# Run

```bash
docker run --rm kmindex-d <kmindex args>
```

The default entrypoint corresponds to `kmindex`. To run `kmindex-server`, use:

```bash
docker run --rm --entrypoint /result/bin/kmindex-server kmindex-d <kmindex-server args>
```
