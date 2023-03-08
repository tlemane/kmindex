# Welcome to kmindex documentation

[![kmindex](https://github.com/tlemane/kmindex/workflows/kmindex/badge.svg)](https://github.com/tlemane/kmindex/actions/workflows/kmindex.yml)
[![release](https://img.shields.io/github/v/release/tlemane/kmindex)](https://github.com/tlemane/kmindex/releases)
[![dockerhub](https://img.shields.io/docker/v/tlemane/kmindex?label=tlemane/kmindex&logo=docker)](https://hub.docker.com/r/tlemane/kmindex/)
[![anaconda](https://img.shields.io/conda/vn/tlemane/kmindex?color=green&label=tlemane%2Fkmindex&logo=anaconda)](https://anaconda.org/tlemane/kmindex)

*kmindex* is a tool for indexing and querying sequencing samples. It is built on top of kmtricks](https://github.com/tlemane/kmtricks) modules.

Given a databank $D = \{S_1, ..., S_n\}$, with each $S_i$ being any genomic dataset (genome or raw reads), *kmindex* allows to compute the percentage of shared k-mers between a query $Q$ and each $S \in D$. It supports multiple datasets and allows searching for each sub-index $D_i \in G = \{D_1,...,D_m\}$. Queries benefit from the [findere](https://github.com/lrobidou/findere) algorithm. In a few words, *findere* allows to reduce the false positive rate at query time by querying $(s+z)$-mers instead of $s$-mers, which are the indexed words, usually called $k$-mers.

## User guide

* [Installation](installation.md)
* [Index construction](construction.md)
* [Index query](query.md)

## Server

* [Deploy](server-deploy.md)
* [Query](server-query.md)


!!! info "Contact"
    Teo Lemane: teo[dot]lemane[at]proton[dot]me

