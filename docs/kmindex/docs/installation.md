# kmindex installation

## **Conda** [![anaconda](https://img.shields.io/conda/vn/tlemane/kmindex?color=green&label=tlemane%2Fkmindex&logo=anaconda)](https://anaconda.org/tlemane/kmindex)

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

[Download latest release](https://github.com/tlemane/kmindex/releases/latest/download/kmindex-v0.1.0-bin-Linux.tar.gz){ .md-button .md-button--primary }

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
    kmindex build script.
    Usage:
      ./install.sh [-r str] [-t int] [-j int] [-p str] [-n] [-h]
    Options:
      -r <Release|Debug> -> build type {Release}.
      -t <0|1|2>         -> tests: 0 = disabled, 1 = compile, 2 = compile and run {2}.
      -j <INT>           -> nb threads {8}.
      -n                 -> portable x86-64 build {disable}.
      -p                 -> install path {./kmindex_install}
      -h                 -> show help.
    ```
