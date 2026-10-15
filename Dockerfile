# syntax=docker/dockerfile:1

# ---------------------------------------------------------------------------
# base: pinned toolchain (Ubuntu 22.04, Clang/LLVM 15) + Flex/Bison + CMake
# ---------------------------------------------------------------------------
FROM ubuntu:22.04 AS base

ENV DEBIAN_FRONTEND=noninteractive
ENV LLVM_VERSION=15

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    gnupg \
    lsb-release \
    wget \
    git \
    build-essential \
    cmake \
    ninja-build \
    flex \
    bison \
    python3 \
    sudo \
    && rm -rf /var/lib/apt/lists/*

RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor > /etc/apt/trusted.gpg.d/llvm.gpg \
    && echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-${LLVM_VERSION} main" > /etc/apt/sources.list.d/llvm.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
    llvm-${LLVM_VERSION} \
    llvm-${LLVM_VERSION}-dev \
    llvm-${LLVM_VERSION}-tools \
    clang-${LLVM_VERSION} \
    libclang-${LLVM_VERSION}-dev \
    lld-${LLVM_VERSION} \
    && rm -rf /var/lib/apt/lists/*

ENV CC=clang-${LLVM_VERSION}
ENV CXX=clang++-${LLVM_VERSION}
ENV LLVM_CONFIG=/usr/bin/llvm-config-${LLVM_VERSION}

# Non-root development user.
RUN useradd -m -s /bin/bash developer \
    && echo "developer ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER developer
WORKDIR /home/developer/project

# ---------------------------------------------------------------------------
# dev: interactive shell with the source bind-mounted by docker-compose
# ---------------------------------------------------------------------------
FROM base AS dev
CMD ["/bin/bash"]

# ---------------------------------------------------------------------------
# test: copy the source, configure, build the compiler and run the test suite.
# The build artifacts are kept in the image so `docker run <image>` can re-run
# ctest without rebuilding.
# ---------------------------------------------------------------------------
FROM base AS test
COPY --chown=developer:developer . /home/developer/project

RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build \
    && ctest --test-dir build --output-on-failure

CMD ["ctest", "--test-dir", "build", "--output-on-failure"]
