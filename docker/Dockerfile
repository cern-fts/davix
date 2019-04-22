FROM debian:stretch-slim
# The default is to build the development branch, set this to a release tag such
# as R_0_7_2 to get a Docker image for that version.
ARG DAVIX_RELEASE=devel

RUN \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        git ca-certificates cmake g++ build-essential \
        libxml2-dev libssl-dev uuid-dev python && \
    git clone --branch "$DAVIX_RELEASE" https://github.com/cern-fts/davix.git && \
    cd davix && \
    git submodule update --recursive --init && \
    mkdir build && cd build && \
    cmake -Wno-dev .. && \
    make && \
    make install && \
    ldconfig && \
    DEBIAN_FRONTEND=noninteractive apt-get remove --purge -y \
        git cmake g++ libxml2-dev libssl-dev build-essential uuid-dev python && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        libxml2 && \
    DEBIAN_FRONTEND=noninteractive apt-get autoremove -y && \
    apt-get clean -y && \
    rm -rf /var/lib/apt/lists/* && \
    cd ../.. && \
    rm -rf davix
COPY *.sh /usr/local/bin/
ENTRYPOINT [ "/usr/local/bin/davix.sh" ]