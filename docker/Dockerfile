FROM ubuntu:jammy

ENV DEBIAN_FRONTEND=noninteractive

ENV KMINDEX_VERSION v0.5.3

ENV BUILD_INPUTS git cmake gcc g++ zlib1g zlib1g-dev libboost-all-dev make \
                 libspdlog-dev libspdlog1 bzip2 lbzip2 xxhash ca-certificates libbz2-dev

WORKDIR /home

RUN apt-get update \
    && apt-get -y dist-upgrade \
    && apt-get install -y --no-install-recommends wget ${BUILD_INPUTS} \
    && apt-get clean

RUN wget https://github.com/Kitware/CMake/releases/download/v3.26.4/cmake-3.26.4-linux-x86_64.tar.gz \
    && tar -xzvf cmake-3.26.4-linux-x86_64.tar.gz

RUN cd /home \
    && mkdir kmindex \
    && cd kmindex \
    && wget https://github.com/tlemane/kmindex/releases/download/${KMINDEX_VERSION}/kmindex-${KMINDEX_VERSION}-sources.tar.gz \
    && tar -xzvf kmindex-${KMINDEX_VERSION}-sources.tar.gz

RUN cd /home \
    && cd kmindex \
    && rm -rf build \
    && mkdir -p build \
    && cd build \
    && /home/cmake-3.26.4-linux-x86_64/bin/cmake .. -DPORTABLE_BUILD=ON \
    && make -j8 && make install -p /bin

RUN rm -rf /home/kmindex /home/cmake-3.26.4-linux-x86_64*


WORKDIR /tmp

ENTRYPOINT ["kmindex"]

LABEL version="0.5.3"
LABEL description="kmindex"
LABEL author="Teo Lemane"

