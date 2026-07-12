# syntax=docker/dockerfile:1.7

FROM ubuntu:24.04 AS builder

ARG TARGETARCH
ARG VCPKG_COMMIT=82216afaef8cccfd2a395c839b807801f366879e

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      autoconf \
      automake \
      autoconf-archive \
      bison \
      build-essential \
      ca-certificates \
      curl \
      flex \
      git \
      libtool \
      ninja-build \
      pkg-config \
      python3 \
      tar \
      unzip \
      zip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git . \
    && git checkout "${VCPKG_COMMIT}" \
    && ./bootstrap-vcpkg.sh -disableMetrics

WORKDIR /src
COPY vcpkg.json ./

RUN case "${TARGETARCH}" in \
      amd64) triplet=x64-linux-dynamic ;; \
      arm64) triplet=arm64-linux-dynamic ;; \
      *) echo "Unsupported TARGETARCH: ${TARGETARCH}" >&2; exit 1 ;; \
    esac \
    && /opt/vcpkg/vcpkg install \
      --triplet "${triplet}" \
      --x-install-root=/opt/vcpkg/installed

COPY CMakeLists.txt ./
COPY src ./src

RUN case "${TARGETARCH}" in \
      amd64) triplet=x64-linux-dynamic ;; \
      arm64) triplet=arm64-linux-dynamic ;; \
      *) echo "Unsupported TARGETARCH: ${TARGETARCH}" >&2; exit 1 ;; \
    esac \
    && cmake -S . -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_INSTALLED_DIR=/opt/vcpkg/installed \
      -DVCPKG_TARGET_TRIPLET="${triplet}" \
    && cmake --build build --parallel \
    && mkdir -p /opt/runtime \
    && cp -a "/opt/vcpkg/installed/${triplet}/lib" /opt/runtime/lib \
    && cp -a "/opt/vcpkg/installed/${triplet}/share/wt" /opt/runtime/wt

FROM ubuntu:24.04 AS runtime

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      ca-certificates \
      curl \
      libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/kopds
COPY --from=builder /src/build/kopds-app ./bin/kopds-app
COPY --from=builder /opt/runtime/lib ./lib
COPY --from=builder /opt/runtime/wt ./share/wt

ENV LD_LIBRARY_PATH=/opt/kopds/lib

EXPOSE 8080

HEALTHCHECK --interval=10s --timeout=3s --start-period=10s --retries=3 \
  CMD curl --fail --silent http://127.0.0.1:8080/ >/dev/null || exit 1

ENTRYPOINT ["/opt/kopds/bin/kopds-app"]
CMD ["--http-address", "0.0.0.0", "--http-port", "8080", "--docroot", ".", "--resources-dir", "/opt/kopds/share/wt/resources"]
