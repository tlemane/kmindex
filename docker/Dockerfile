FROM ubuntu:jammy

ENV DEBIAN_FRONTEND=noninteractive

ENV KMINDEX_VERSION v0.2.0

ENV BUILD_INPUTS git cmake gcc g++ zlib1g zlib1g-dev libboost-all-dev make \
                 libspdlog-dev libspdlog1 bzip2 lbzip2 xxhash ca-certificates libbz2-dev

WORKDIR /home

RUN apt-get update \
    && apt-get -y dist-upgrade \
    && apt-get install -y --no-install-recommends wget ${BUILD_INPUTS} \
    && apt-get clean

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
    && cmake .. -DPORTABLE_BUILD=ON \
    && make -j8 && make install -p /bin

RUN rm -rf /home/kmindex

WORKDIR /tmp

ENTRYPOINT ["kmindex"]

LABEL version="0.2.0"
LABEL description="kmindex"
LABEL author="Teo Lemane"

