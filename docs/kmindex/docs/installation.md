# kmindex installation

!!! note
    **kmindex** supports GNU/Linux systems, and macOS since v0.5.2.

## **Conda** [![anaconda](https://img.shields.io/conda/vn/tlemane/kmindex?color=green&label=tlemane%2Fkmindex&logo=anaconda)](https://anaconda.org/tlemane/kmindex) ![anaconda-platform](https://img.shields.io/conda/pn/tlemane/kmindex?logo=anaconda)

```bash
conda create -p kmindex_env
conda activate ./kmindex_env
conda install -c conda-forge -c tlemane kmindex
```

Note that the conda package includes [kmtricks](https://github.com/tlemane/kmtricks).

## **Docker** [![dockerhub](https://img.shields.io/docker/v/tlemane/kmindex?label=tlemane/kmindex&logo=docker)](https://hub.docker.com/r/tlemane/kmindex/)

```bash
docker pull tlemane/kmindex
```

## **Nix**

```bash
  nix --extra-experimental-features 'nix-command flakes' build github:tlemane/kmindex
```

## **Release**  [![release](https://img.shields.io/github/v/release/tlemane/kmindex)](https://github.com/tlemane/kmindex/releases)

Portable binary releases are available at [kmindex/releases](https://github.com/tlemane/kmindex/releases).

[:simple-linux:](https://github.com/tlemane/kmindex/releases/latest/download/kmindex-v0.5.2-bin-Linux.tar.gz){ .md-button .md-button--primary } [:simple-apple:](https://github.com/tlemane/kmindex/releases/latest/download/kmindex-v0.5.2-bin-Darwin.tar.gz){ .md-button .md-button--primary }


## **Install from sources**

!!! info "**Requirements**"
    * cmake >= 3.13
    * gcc >= 8.1
    * Boost.Asio and Boost.System >= 1.70
    * bzip2

    All other dependencies are bundled with `kmindex`.

**Installation**
``` bash
git clone --recursive https://github.com/tlemane/kmindex # (1)!
cd kmindex && ./install.sh # (2)!
```

1. Cloning the repository
2. Build kmindex using `install.sh`

!!! tip "`install.sh`"
    ```
    kmindex build script - v0.5.0.
    Usage:
      ./install.sh [-r str] [-t int] [-j int] [-p str] [-k int] [-n] [-h]
    Options:
      -r <Release|Debug> -> build type {Release}.
      -t <0|1|2>         -> tests: 0 = disabled, 1 = compile, 2 = compile and run {0}.
      -j <INT>           -> nb threads {8}.
      -k <INT>           -> Max k-mer size (should be a multiple of 32) {256}
      -n                 -> portable x86-64 build {disabled}.
      -p                 -> install path {./kmindex_install}
      -h                 -> show help.
    ```
